#pragma once

#include <vulkan/vulkan.h>
#include "String.h"
#include "VulkanFence.h"
#include "CommandBuffer.h"
//NOTE: In pngrutil.c I made a note!
#include <png.h>
#include "Ifstream.h"

class VulkanTexture
{
    struct PngReadInfo
    {
        png_structp png = nullptr;
        png_infop info = nullptr;
        uint32_t width = 0;
        uint32_t height = 0;
        VkFormat format = VK_FORMAT_UNDEFINED;
    };
private:
    uint32_t width = 0;
    uint32_t height = 0;
    VkImage texture = VK_NULL_HANDLE;
    VkImageView textureView = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;

    //TODO: Think about not creating a commandbuffer for every texture?!
	CommandBuffer commandBuffer;
	VulkanFence fence;

	static const class GraphicsVKIniter* gfx;

    uint32_t mipLevels = 1;
public:
    enum class TextureType
    {
        NORMAL,
        CUBE_MAP,
    };
private:
    TextureType textureType = TextureType::NORMAL;
public:
    void reloadFromFile(const String& filename);
public:
    VulkanTexture() = default;
    VulkanTexture(const String& filename, TextureType textureType = TextureType::NORMAL);
    VulkanTexture(const void* buffer, uint32_t width, uint32_t height);
	VulkanTexture(VkImageCreateInfo& imageCreateInfo, VkMemoryPropertyFlagBits memoryPropertyFlags, bool setDefaultGraphicsFamilyIndex,
        VkImageAspectFlags aspectFlag);
    VulkanTexture(const VulkanTexture& other) = delete;
    VulkanTexture(VulkanTexture&& other) noexcept;
    VulkanTexture& operator=(const VulkanTexture& rhs) = delete;
    VulkanTexture& operator=(VulkanTexture&& rhs) noexcept;
    ~VulkanTexture();
    uint32_t getWidth() const { return width; }
    uint32_t getHeight() const { return height; }
    uint64_t getSize() const { return (((uint64_t)width) * height * 4 * mipLevels + sizeof(VulkanTexture)); }
    explicit operator bool() const;
    const VkImageView getTextureView() const { return textureView; }
	const VkImage getTexture() const { return texture; }
	void changeVkImageLayout(VkImageMemoryBarrier& imageMemoryBarrier, VkPipelineStageFlags srcStage,
		VkPipelineStageFlags dstStage, bool setDefaultGraphicsFamilyIndex);
    static VulkanTexture createDepthTexture(uint32_t width, uint32_t height);
private:
    static VulkanTexture createCubeMap(const LongString& dirName);
	VkDeviceSize createVkImageAndView(const VkImageCreateInfo& imageCreateInfo,
		VkMemoryPropertyFlagBits memoryPropertyFlags, VkImageViewType viewType,
        VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT);
    uint8_t* createAndMap(uint32_t graphicsFamilyIndex, VkFormat internalFormat, class VulkanBuffer* stagingBuffer);
    void unmapAndFinish(class VulkanBuffer* stagingBuffer);
    PngReadInfo startPngLoading(Ifstream& asset);
    void finishPngLoading(PngReadInfo& readInfo, png_byte* image);
    void copyStagingBufferToDeviceLocalMemory(VulkanBuffer* stagingBuffer, uint32_t layerCount);
    void createMipMaps();
    void convertToOptimalSampling(uint32_t layerCount);
    void setGfx() const;
};
