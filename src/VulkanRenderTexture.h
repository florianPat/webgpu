#pragma once

#include "Types.h"
#include "VulkanTexture.h"
#include "Mat4x4.h"
#include "GraphicsVK.h"

class VulkanRenderTexture
{
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VulkanTexture texture;
    VulkanTexture depthTexture;
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	CommandBuffer commandBuffer;
	VulkanFence fence;
    VkClearValue clearValues[2] = {};
    VkClearAttachment clearAttachments[2] = {};
    VkClearRect clearRects[2] = {};

	GraphicsVKIniter* gfx = nullptr;

	static VkRenderPass renderPass;
	VkPipeline pipeline;
public:
    VulkanRenderTexture() = default;
    VulkanRenderTexture(uint32_t width, uint32_t height, GraphicsVKIniter* gfx, uint32_t rendererId);
    VulkanRenderTexture(const VulkanRenderTexture& other) = delete;
    VulkanRenderTexture(VulkanRenderTexture&& other) noexcept;
    VulkanRenderTexture& operator=(const VulkanRenderTexture& rhs) = delete;
    VulkanRenderTexture& operator=(VulkanRenderTexture&& rhs) noexcept;
    ~VulkanRenderTexture();
    const VulkanTexture& getTexture() const;
    FramebufferDrawStruct getFramebufferDrawStruct();
    explicit operator bool() const;
    void clear();
	void changeToShaderReadMode();
    void changeToShaderWriteMode();
    static void disposeRenderPass(VkDevice device);
private:
	void createFramebufferTextureAndView(uint32_t width, uint32_t height, VkFormat surfaceFormat);
    void setClearValues();
};
