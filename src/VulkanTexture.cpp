#include "VulkanTexture.h"
#include "CommandBuffer.h"
#include "VulkanFence.h"
#include "GraphicsVKIniter.h"
#include "VulkanBuffer.h"
#include "Globals.h"
#include "Window.h"

const GraphicsVKIniter* VulkanTexture::gfx = nullptr;

VulkanTexture::operator bool() const
{
    return (texture != VK_NULL_HANDLE);
}

VulkanTexture::~VulkanTexture()
{
	fence.~VulkanFence();

    if(textureView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(gfx->device, textureView, nullptr);
        textureView = VK_NULL_HANDLE;
    }
    if(texture != VK_NULL_HANDLE)
    {
        vkDestroyImage(gfx->device, texture, nullptr);
        texture = VK_NULL_HANDLE;
    }
    if (memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(gfx->device, memory, nullptr);
        memory = VK_NULL_HANDLE;
    }
}

VulkanTexture::VulkanTexture(VulkanTexture&& other) noexcept : width(std::exchange(other.width, 0)),
	height(std::exchange(other.height, 0)), texture(std::exchange(other.texture, (VkImage)VK_NULL_HANDLE)),
	textureView(std::exchange(other.textureView, (VkImageView)VK_NULL_HANDLE)),
    memory(std::exchange(other.memory, (VkDeviceMemory)VK_NULL_HANDLE)),
	commandBuffer(std::move(other.commandBuffer)),
	fence(std::move(other.fence)),
    mipLevels(other.mipLevels)
{
}

VulkanTexture& VulkanTexture::operator=(VulkanTexture&& rhs) noexcept
{
    this->~VulkanTexture();

    new (this) VulkanTexture(std::move(rhs));

    return *this;
}

static void callbackReadPng(png_structp pngStruct, png_bytep data, png_size_t size)
{
	Ifstream* asset = ((Ifstream*)png_get_io_ptr(pngStruct));
	asset->read(data, (uint32_t)size);
}

static void callbackReadTransform(png_structp ptr, png_row_infop rowInfo, png_bytep data)
{
	assert(rowInfo->bit_depth == 8); // 1 byte, uchar
	assert(rowInfo->color_type == PNG_COLOR_TYPE_RGBA || rowInfo->color_type == PNG_COLOR_TYPE_RGB);
	assert(rowInfo->channels == 4); //RGBA
	assert(rowInfo->pixel_depth == (4 * 8));

    for (uint32_t i = 0; i < rowInfo->rowbytes; i += 4)
    {
        float alpha = data[i + 3] / 255.0f;

        data[i + 0] = (uint8_t)(data[i + 0] / 255.0f * alpha * 255.0f);
        data[i + 1] = (uint8_t)(data[i + 1] / 255.0f * alpha * 255.0f);
        data[i + 2] = (uint8_t)(data[i + 2] / 255.0f * alpha * 255.0f);
    }
}

VulkanTexture::VulkanTexture(const String& filename, TextureType textureType) : textureType(textureType)
{
    setGfx();

    if (textureType == TextureType::CUBE_MAP)
    {
        // NOTE: This is necessary because of the asset manager...
        assert(filename.find(".png") == (filename.size() - 4));
        String noExtensionName = filename.substr(0, filename.size() - 4);
        *this = createCubeMap(noExtensionName);
        return;
    }

    Ifstream asset;
    asset.open(filename);

    if (!asset)
        InvalidCodePath;

    PngReadInfo readInfo = startPngLoading(asset);
    width = readInfo.width;
    height = readInfo.height;
    mipLevels = (uint32_t)floorf(log2f(width > height ? (float)width : (float)height)) + 1;

    VulkanBuffer stagingBuffer;
	png_byte* image = (png_byte*) createAndMap(gfx->graphicsQueueFamilyIndex, readInfo.format, &stagingBuffer);
    finishPngLoading(readInfo, image);

    unmapAndFinish(&stagingBuffer);

    asset.close();
}

VulkanTexture::VulkanTexture(const void* buffer, uint32_t width, uint32_t height)
        : width(width), height(height), mipLevels((uint32_t)floorf(log2f(width > height ? (float)width : (float)height)) + 1)
{
    setGfx();

    VulkanBuffer stagingBuffer;
    uint8_t* mappedData = createAndMap(gfx->graphicsQueueFamilyIndex, VK_FORMAT_R8G8B8A8_UNORM, &stagingBuffer);
	if (buffer != nullptr)
	{
		const uint8_t* storeData = (const uint8_t*)buffer;
		uint32_t rowSize = width * 4;

		for (uint32_t y = 0; y < height; ++y)
		{
			for (uint32_t x = 0; x < rowSize; ++x)
			{
				mappedData[y * rowSize + x] = storeData[y * rowSize + x];
			}
		}
	}

    unmapAndFinish(&stagingBuffer);
}

VulkanTexture::VulkanTexture(VkImageCreateInfo& imageCreateInfo, VkMemoryPropertyFlagBits memoryPropertyFlags,
	bool setDefaultGraphicsFamilyIndex, VkImageAspectFlags aspectFlag)
    : width(imageCreateInfo.extent.width), height(imageCreateInfo.extent.height), mipLevels(imageCreateInfo.mipLevels)
{
    setGfx();

	if (setDefaultGraphicsFamilyIndex)
	{
		imageCreateInfo.queueFamilyIndexCount = 1;
		imageCreateInfo.pQueueFamilyIndices = &gfx->graphicsQueueFamilyIndex;
	}

	createVkImageAndView(imageCreateInfo, memoryPropertyFlags, VK_IMAGE_VIEW_TYPE_2D, aspectFlag);
}

VkDeviceSize VulkanTexture::createVkImageAndView(const VkImageCreateInfo& imageCreateInfo, VkMemoryPropertyFlagBits memoryPropertyFlags,
    VkImageViewType viewType, VkImageAspectFlags aspectFlag)
{
	new (&commandBuffer) CommandBuffer(gfx->device, gfx->commandPool);
	new (&fence) VulkanFence(gfx->device);

	vkCreateImage(gfx->device, &imageCreateInfo, nullptr, &texture);

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(gfx->device, texture, &memoryRequirements);
	uint32_t memoryTypeIndex = gfx->getMemoryTypeIndex(memoryRequirements.memoryTypeBits, memoryPropertyFlags);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

	vkAllocateMemory(gfx->device, &memoryAllocateInfo, nullptr, &memory);
	assert(((uint64_t)width * height) <= memoryRequirements.size);

	vkBindImageMemory(gfx->device, texture, memory, 0);

	//NOTE: Is defined twice (not really cool!)
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.levelCount = mipLevels;
	subresourceRange.layerCount = imageCreateInfo.arrayLayers;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.aspectMask = aspectFlag;

	VkComponentMapping componentMapping = {};
	componentMapping.r = VK_COMPONENT_SWIZZLE_R;
	componentMapping.g = VK_COMPONENT_SWIZZLE_G;
	componentMapping.b = VK_COMPONENT_SWIZZLE_B;
	componentMapping.a = VK_COMPONENT_SWIZZLE_A;

	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.pNext = nullptr;
	imageViewCreateInfo.flags = 0;
	imageViewCreateInfo.format = imageCreateInfo.format;
	imageViewCreateInfo.image = texture;
	imageViewCreateInfo.viewType = viewType;
	imageViewCreateInfo.subresourceRange = subresourceRange;
	imageViewCreateInfo.components = componentMapping;

	vkCreateImageView(gfx->device, &imageViewCreateInfo, nullptr, &textureView);

	return memoryRequirements.size;
}

uint8_t* VulkanTexture::createAndMap(uint32_t graphicsFamilyIndex, VkFormat internalFormat, VulkanBuffer* stagingBuffer)
{
    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext = nullptr;
    imageCreateInfo.flags = 0;
    imageCreateInfo.format = internalFormat;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.queueFamilyIndexCount = 1;
    imageCreateInfo.pQueueFamilyIndices = &graphicsFamilyIndex;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.mipLevels = mipLevels;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent = VkExtent3D{ width, height, 1 };

	uint64_t memoryRequirementsSize = createVkImageAndView(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_VIEW_TYPE_2D);

    new (stagingBuffer) VulkanBuffer(memoryRequirementsSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, *gfx);
    return stagingBuffer->map(0, VK_WHOLE_SIZE);
}

void VulkanTexture::unmapAndFinish(VulkanBuffer* stagingBuffer)
{
    stagingBuffer->unmap(0, VK_WHOLE_SIZE);

    // TODO: Optimize!
    copyStagingBufferToDeviceLocalMemory(stagingBuffer, 1);

    createMipMaps();

    convertToOptimalSampling(1);
}

VulkanTexture::PngReadInfo VulkanTexture::startPngLoading(Ifstream& asset)
{
    png_byte header[8];
    png_structp png = nullptr;
    png_infop info = nullptr;

    asset.read(header, sizeof(header));
    if (png_sig_cmp(header, 0, 8) != 0)
        InvalidCodePath;

    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png)
        InvalidCodePath;
    info = png_create_info_struct(png);
    if (!info)
    {
        png_destroy_read_struct(&png, nullptr, nullptr);
        InvalidCodePath;
    }

    png_set_read_fn(png, &asset, callbackReadPng);
    png_set_read_user_transform_fn(png, callbackReadTransform);

    png_set_sig_bytes(png, 8);
    png_read_info(png, info);
    png_int_32 depth, colorType;
    png_uint_32 width, height;
    png_get_IHDR(png, info, &width, &height, &depth, &colorType, nullptr, nullptr, nullptr);

    if (png_get_valid(png, info, PNG_INFO_tRNS))
    {
        png_set_tRNS_to_alpha(png);
    }

    if (depth < 8)
    {
        png_set_packing(png);
    }
    else if (depth == 16)
    {
        png_set_strip_16(png);
    }

    VkFormat format = VK_FORMAT_UNDEFINED;
    switch (colorType)
    {
        case PNG_COLOR_TYPE_PALETTE:
        {
            png_set_palette_to_rgb(png);
        }
        case PNG_COLOR_TYPE_RGB:
        {
            png_set_add_alpha(png, 255, PNG_FILLER_AFTER);
        }
        case PNG_COLOR_TYPE_RGBA:
        {
            format = VK_FORMAT_R8G8B8A8_SRGB;
            break;
        }
        default:
        {
            png_destroy_read_struct(&png, &info, nullptr);
            InvalidCodePath;
            break;
        }
    }
    png_read_update_info(png, info);

    return PngReadInfo{ png, info, width, height, format };
}

void VulkanTexture::finishPngLoading(PngReadInfo& readInfo, png_byte* image)
{
    png_size_t rowSize = png_get_rowbytes(readInfo.png, readInfo.info);
    assert(rowSize == width * 4);

    png_bytep* rowPtrs = (png_bytep*)malloc(height * sizeof(png_bytep));

    for (uint32_t i = 0; i < height; ++i)
    {
        rowPtrs[i] = &image[i * rowSize];
    }

    png_read_image(readInfo.png, rowPtrs);
    free(rowPtrs);
    png_destroy_read_struct(&readInfo.png, &readInfo.info, nullptr);
}

void VulkanTexture::copyStagingBufferToDeviceLocalMemory(VulkanBuffer* stagingBuffer, uint32_t layerCount)
{
    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = layerCount;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.pNext = nullptr;
    imageMemoryBarrier.subresourceRange = subresourceRange;
    imageMemoryBarrier.image = texture;
    imageMemoryBarrier.srcQueueFamilyIndex = gfx->graphicsQueueFamilyIndex;
    imageMemoryBarrier.dstQueueFamilyIndex = gfx->graphicsQueueFamilyIndex;

    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    imageMemoryBarrier.srcAccessMask = 0;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    changeVkImageLayout(imageMemoryBarrier, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, false);

    commandBuffer.begin();

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layerCount;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkCmdCopyBufferToImage(commandBuffer.commandBuffer, stagingBuffer->buffer, texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
        &region);
    commandBuffer.end();
    commandBuffer.submit(gfx->queue, fence.fence);
    fence.wait();
    fence.reset();
}

void VulkanTexture::createMipMaps()
{
    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.pNext = nullptr;
    imageMemoryBarrier.subresourceRange = subresourceRange;
    imageMemoryBarrier.image = texture;
    imageMemoryBarrier.srcQueueFamilyIndex = gfx->graphicsQueueFamilyIndex;
    imageMemoryBarrier.dstQueueFamilyIndex = gfx->graphicsQueueFamilyIndex;

    uint32_t mipWidth = width;
    uint32_t mipHeight = height;

    for (uint32_t i = 1; i < mipLevels; ++i)
    {
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = i - 1;
        changeVkImageLayout(imageMemoryBarrier, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, false);

        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = i;
        changeVkImageLayout(imageMemoryBarrier, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, false);

        VkImageSubresourceLayers blitSubressource = {};
        blitSubressource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitSubressource.baseArrayLayer = 0;
        blitSubressource.layerCount = 1;

        VkImageBlit blit = {};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { (int32_t)mipWidth, (int32_t)mipHeight, 1 };
        blit.srcSubresource = blitSubressource;
        blit.srcSubresource.mipLevel = i - 1;

        if (mipWidth > 1)
            mipWidth /= 2;
        if (mipHeight > 1)
            mipHeight /= 2;

        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { (int32_t)mipWidth, (int32_t)mipHeight, 1 };
        blit.dstSubresource = blitSubressource;
        blit.dstSubresource.mipLevel = i;

        commandBuffer.begin();
        vkCmdBlitImage(commandBuffer.commandBuffer, texture, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);
        commandBuffer.end();
        commandBuffer.submit(gfx->queue, fence.fence);
        fence.wait();
        fence.reset();
    }
    assert(mipWidth == 1 && mipHeight == 1);
}

void VulkanTexture::convertToOptimalSampling(uint32_t layerCount)
{
    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.layerCount = layerCount;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.pNext = nullptr;
    imageMemoryBarrier.subresourceRange = subresourceRange;
    imageMemoryBarrier.image = texture;
    imageMemoryBarrier.srcQueueFamilyIndex = gfx->graphicsQueueFamilyIndex;
    imageMemoryBarrier.dstQueueFamilyIndex = gfx->graphicsQueueFamilyIndex;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    if (mipLevels > 1)
    {
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imageMemoryBarrier.subresourceRange.levelCount = mipLevels - 1;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        changeVkImageLayout(imageMemoryBarrier, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, false);
    }

    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.subresourceRange.levelCount = 1;
    imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevels - 1;
    changeVkImageLayout(imageMemoryBarrier, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, false);
}

void VulkanTexture::setGfx() const
{
    if (gfx == nullptr)
    {
        gfx = &Globals::window->getGfx();
    }
}

void VulkanTexture::changeVkImageLayout(VkImageMemoryBarrier& imageMemoryBarrier, VkPipelineStageFlags srcStage,
	VkPipelineStageFlags dstStage, bool setDefaultGraphicsFamilyIndex)
{
	if (setDefaultGraphicsFamilyIndex)
	{
		imageMemoryBarrier.srcQueueFamilyIndex = gfx->graphicsQueueFamilyIndex;
		imageMemoryBarrier.dstQueueFamilyIndex = gfx->graphicsQueueFamilyIndex;
	}

	commandBuffer.begin();

	vkCmdPipelineBarrier(commandBuffer.commandBuffer, srcStage, dstStage,
		0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

	commandBuffer.end();
	commandBuffer.submit(gfx->queue, fence.fence);

	fence.wait();
	fence.reset();
}

VulkanTexture VulkanTexture::createDepthTexture(uint32_t width, uint32_t height)
{
    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext = nullptr;
    imageCreateInfo.flags = 0;
    imageCreateInfo.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = 1;

    VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    VulkanTexture result(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true, aspectFlag);

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.aspectMask = aspectFlag;

    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.pNext = nullptr;
    imageMemoryBarrier.srcAccessMask = 0;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    imageMemoryBarrier.subresourceRange = subresourceRange;
    imageMemoryBarrier.image = result.texture;

    result.changeVkImageLayout(imageMemoryBarrier, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, true);

    return result;
}

VulkanTexture VulkanTexture::createCubeMap(const LongString& dirName)
{
    VulkanTexture result;

    Ifstream asset;
    asset.open(dirName + "/right.png");
    if (!asset)
        InvalidCodePath;

    PngReadInfo readInfo = result.startPngLoading(asset);
    result.width = readInfo.width;
    result.height = readInfo.height;
    result.mipLevels = 1;

    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext = nullptr;
    imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    imageCreateInfo.format = readInfo.format;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.queueFamilyIndexCount = 1;
    imageCreateInfo.pQueueFamilyIndices = &gfx->graphicsQueueFamilyIndex;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 6;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent = VkExtent3D{ readInfo.width, readInfo.height, 1 };

    uint64_t memoryRequirementsSize = result.createVkImageAndView(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_VIEW_TYPE_CUBE);

    VulkanBuffer stagingBuffer(memoryRequirementsSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, *gfx);
    png_byte* image = stagingBuffer.map(0, VK_WHOLE_SIZE);

    result.finishPngLoading(readInfo, image);

    String cubeMapNames[] = { dirName + "/left.png", dirName + "/top.png", dirName + "/bottom.png", dirName + "/front.png",
        dirName + "/back.png" };

    for (uint32_t i = 0; i < arrayCount(cubeMapNames); ++i)
    {
        asset.close();
        
        asset.open(cubeMapNames[i]);
        if (!asset)
            InvalidCodePath;

        readInfo = result.startPngLoading(asset);
        assert(readInfo.width == result.width && readInfo.height == result.height);
        result.finishPngLoading(readInfo, &image[readInfo.width * readInfo.height * 4 * (i + 1)]);
    }
    asset.close();

    stagingBuffer.unmap(0, VK_WHOLE_SIZE);
    result.copyStagingBufferToDeviceLocalMemory(&stagingBuffer, 6);
    result.convertToOptimalSampling(6);

    return result;
}

void VulkanTexture::reloadFromFile(const String& filename)
{
    this->~VulkanTexture();

    new (this) VulkanTexture(filename, textureType);
}
