#pragma once

#include <vulkan/vulkan.h>
#include "Types.h"
#include "CommandBuffer.h"
#include "GraphicsVKIniter.h"

class VulkanBuffer
{
    VkDevice device = nullptr;
    uint64_t size = 0;
public:
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
public:
    VulkanBuffer() = default;
    VulkanBuffer(uint64_t size, VkBufferUsageFlags usageFlag, const GraphicsVKIniter& gfx);
    VulkanBuffer(const VulkanBuffer& other) = delete;
    VulkanBuffer(VulkanBuffer&& other) noexcept;
    VulkanBuffer& operator=(const VulkanBuffer& rhs) = delete;
    VulkanBuffer& operator=(VulkanBuffer&& rhs) noexcept;
    ~VulkanBuffer();
    explicit operator bool() const;
    void bindVertexBuffer(VkCommandBuffer commandBuffer, uint32_t bindingIndex);
    void bindIndexBuffer(VkCommandBuffer commandBuffer);
	void cmdSubData(const CommandBuffer& commandBuffer, uint32_t offset, uint32_t size, const void* data);
    uint64_t getSize() const;
    uint8_t* map(uint32_t offset, uint64_t size);
    void unmap(uint32_t offset, uint64_t size);
};
