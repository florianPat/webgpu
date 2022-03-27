#include "VulkanRenderTexture.h"
#include "GraphicsVKIniter.h"

VkRenderPass VulkanRenderTexture::renderPass = (VkRenderPass)VK_NULL_HANDLE;

VulkanRenderTexture::VulkanRenderTexture(VulkanRenderTexture&& other) noexcept
	: framebuffer(std::exchange(other.framebuffer, (VkFramebuffer)VK_NULL_HANDLE)),
    texture(std::move(other.texture)), depthTexture(std::move(other.depthTexture)),
	commandBuffer(std::move(other.commandBuffer)), fence(std::move(other.fence)),
	pipeline(std::exchange(other.pipeline, (VkPipeline)VK_NULL_HANDLE)),
	renderPassBeginInfo(std::move(other.renderPassBeginInfo))
{
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = framebuffer;

	setClearValues();
}

VulkanRenderTexture& VulkanRenderTexture::operator=(VulkanRenderTexture&& rhs) noexcept
{
    this->~VulkanRenderTexture();

    new (this) VulkanRenderTexture(std::move(rhs));

    return *this;
}

VulkanRenderTexture::~VulkanRenderTexture()
{
	fence.~VulkanFence();

	if (pipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(gfx->device, pipeline, nullptr);
		pipeline = VK_NULL_HANDLE;
	}

	if (framebuffer != VK_NULL_HANDLE)
	{
		vkDestroyFramebuffer(gfx->device, framebuffer, nullptr);
		framebuffer = VK_NULL_HANDLE;
	}

	commandBuffer.~CommandBuffer();
    texture.~VulkanTexture();
	depthTexture.~VulkanTexture();
}

const VulkanTexture& VulkanRenderTexture::getTexture() const
{
    return texture;
}

FramebufferDrawStruct VulkanRenderTexture::getFramebufferDrawStruct()
{
	return FramebufferDrawStruct{ &commandBuffer, &fence, &renderPassBeginInfo, pipeline };
}

VulkanRenderTexture::operator bool() const
{
    return (framebuffer != VK_NULL_HANDLE);
}

void VulkanRenderTexture::clear()
{
	commandBuffer.begin();
	commandBuffer.beginRenderPass(&renderPassBeginInfo);

	vkCmdClearAttachments(commandBuffer.commandBuffer, 2, clearAttachments, 2, clearRects);

	commandBuffer.endRenderPass();
	commandBuffer.end();

	commandBuffer.submit(gfx->queue, VK_NULL_HANDLE, VK_NULL_HANDLE, fence.fence);

	fence.wait();
	fence.reset();
}

void VulkanRenderTexture::changeToShaderReadMode()
{
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = 1;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//To convert it to an optimized layout the gpu understands
	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = nullptr;
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarrier.subresourceRange = subresourceRange;
	imageMemoryBarrier.image = texture.getTexture();
	texture.changeVkImageLayout(imageMemoryBarrier, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		true);
}

void VulkanRenderTexture::changeToShaderWriteMode()
{
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = 1;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//To convert it to an optimized layout the gpu understands
	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = nullptr;
	imageMemoryBarrier.srcAccessMask = 0;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imageMemoryBarrier.subresourceRange = subresourceRange;
	imageMemoryBarrier.image = texture.getTexture();
	texture.changeVkImageLayout(imageMemoryBarrier, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		true);
}

void VulkanRenderTexture::disposeRenderPass(VkDevice device)
{
	if (renderPass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(device, renderPass, nullptr);
		renderPass = VK_NULL_HANDLE;
	}
}

void VulkanRenderTexture::createFramebufferTextureAndView(uint32_t width, uint32_t height, VkFormat surfaceFormat)
{
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.flags = 0;
	imageCreateInfo.format = surfaceFormat;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent = VkExtent3D{ width, height, 1 };
	new (&texture) VulkanTexture(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true, VK_IMAGE_ASPECT_COLOR_BIT);

	changeToShaderWriteMode();

	depthTexture = VulkanTexture::createDepthTexture(width, height);
}

void VulkanRenderTexture::setClearValues()
{
	for (uint32_t i = 0; i < 4; ++i)
	{
		clearValues[0].color.float32[i] = 0.0f;
	}
	clearValues[1].depthStencil.depth = 1.0f;
	clearValues[1].depthStencil.stencil = 0;

	clearAttachments[0].colorAttachment = 0;
	clearAttachments[0].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	clearAttachments[0].clearValue = clearValues[0];
	clearAttachments[1].colorAttachment = 1;
	clearAttachments[1].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	clearAttachments[1].clearValue = clearValues[1];

	clearRects[0].baseArrayLayer = 0;
	clearRects[0].layerCount = 1;
	clearRects[0].rect.offset = VkOffset2D{ 0, 0 };
	clearRects[0].rect.extent = renderPassBeginInfo.renderArea.extent;
	clearRects[1].baseArrayLayer = 0;
	clearRects[1].layerCount = 1;
	clearRects[1].rect.offset = VkOffset2D{ 0, 0 };
	clearRects[1].rect.extent = renderPassBeginInfo.renderArea.extent;
}

VulkanRenderTexture::VulkanRenderTexture(uint32_t width, uint32_t height, GraphicsVKIniter* gfxIn, uint32_t rendererId) : gfx(gfxIn)
{
	if(renderPass == VK_NULL_HANDLE)
		renderPass = gfx->createRenderPass(gfx->getSurfaceFormat(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	GraphicsVK* renderer = gfx->getRenderer(rendererId);
	pipeline = gfx->createPipeline(renderer->getShader(), renderer->getDescriptorSetLayout(), renderPass, { width, height }, false, renderer->getPipeline());

	createFramebufferTextureAndView(width, height, gfx->getSurfaceFormat());
	VkImageView textureViews[2] = { texture.getTextureView(), depthTexture.getTextureView() };

    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = nullptr;
    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.width = width;
    framebufferCreateInfo.height = height;
    framebufferCreateInfo.flags = 0;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.attachmentCount = 2;
    framebufferCreateInfo.pAttachments = textureViews;
    vkCreateFramebuffer(gfx->device, &framebufferCreateInfo, nullptr, &framebuffer);

	new (&commandBuffer) CommandBuffer(gfx->device, gfx->commandPool);
	new (&fence) VulkanFence(gfx->device);

	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.clearValueCount = 0;
	renderPassBeginInfo.pClearValues = nullptr;
	renderPassBeginInfo.renderArea.offset = VkOffset2D{0, 0};
	renderPassBeginInfo.renderArea.extent = VkExtent2D{ width, height };
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = framebuffer;

	setClearValues();
}
