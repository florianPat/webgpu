#include "VulkanFence.h"
#include "Utils.h"
#include <utility>
#include <new>

VulkanFence::VulkanFence(VkDevice device) : device(device)
{
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.pNext = nullptr;
	fenceCreateInfo.flags = 0;

	vkCreateFence(device, &fenceCreateInfo, nullptr, &fence);
}

VulkanFence::VulkanFence(VulkanFence&& other) noexcept : fence(std::exchange(other.fence, nullptr)),
														 device(std::exchange(other.device, nullptr))
{
}

VulkanFence& VulkanFence::operator=(VulkanFence&& other) noexcept
{
	this->~VulkanFence();

	new (this) VulkanFence(std::move(other));

	return *this;
}

VulkanFence::~VulkanFence()
{
	if (fence != VK_NULL_HANDLE)
	{
		vkDestroyFence(device, fence, nullptr);
		fence = VK_NULL_HANDLE;
		device = nullptr;
	}
}

void VulkanFence::wait() const
{
	VkResult result = vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
	assert(result == VK_SUCCESS);
}

void VulkanFence::reset() const
{
	vkResetFences(device, 1, &fence);
}
