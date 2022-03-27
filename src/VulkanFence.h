#pragma once

#include <vulkan/vulkan.h>

struct VulkanFence
{
	VkFence fence = VK_NULL_HANDLE;
private:
	VkDevice device = VK_NULL_HANDLE;
public:
	VulkanFence() = default;
	VulkanFence(VkDevice device);
	VulkanFence(const VulkanFence& other) = delete;
	VulkanFence& operator=(const VulkanFence& other) = delete;
	VulkanFence(VulkanFence&& other) noexcept;
	VulkanFence& operator=(VulkanFence&& other) noexcept;
	~VulkanFence();
	void wait() const;
	void reset() const;
};