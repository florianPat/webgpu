#pragma once

#include <vulkan/vulkan.h>

class CommandBuffer
{
    VkDevice device = nullptr;
    VkCommandPool commandPool = VK_NULL_HANDLE;
public:
    VkCommandBuffer commandBuffer = nullptr;
public:
    CommandBuffer() = default;
    CommandBuffer(VkDevice device, VkCommandPool commandPool);
    CommandBuffer(const CommandBuffer& other) = delete;
    CommandBuffer(CommandBuffer&& other) noexcept;
    CommandBuffer& operator=(const CommandBuffer& rhs) = delete;
    CommandBuffer& operator=(CommandBuffer&& rhs) noexcept;
    ~CommandBuffer();
    explicit operator bool() const;
    void begin();
    void end();
    void beginRenderPass(VkRenderPassBeginInfo* beginInfo);
    void endRenderPass();
    void submit(VkQueue queue, VkFence fence);
	void submit(VkQueue queue, VkSemaphore drawingCompleteSemaphore, VkSemaphore presentCompleteSemaphore, VkFence fence = VK_NULL_HANDLE);
};
