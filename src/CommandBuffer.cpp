#include "CommandBuffer.h"
#include <utility>
#include <vulkan/vulkan.h>
#include <new>

CommandBuffer::CommandBuffer(VkDevice device, VkCommandPool commandPool) : device(device), commandPool(commandPool)
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.commandBufferCount = 1;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer);
}

CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept : device(std::exchange(other.device, nullptr)),
    commandPool(std::exchange(other.commandPool, nullptr/*VK_NULL_HANDLE*/)), commandBuffer(std::exchange(other.commandBuffer, nullptr))
{
}

CommandBuffer& CommandBuffer::operator=(CommandBuffer&& rhs) noexcept
{
    this->~CommandBuffer();

    new (this) CommandBuffer(std::move(rhs));

    return *this;
}

CommandBuffer::~CommandBuffer()
{
	//NOTE: Command buffers get freed when the commandPool gets freed
	commandBuffer = nullptr;
	device = nullptr;
	commandPool = 0;
}

CommandBuffer::operator bool() const
{
    return (commandBuffer != nullptr);
}

void CommandBuffer::begin()
{
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;
    commandBufferBeginInfo.flags = 0;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
}

void CommandBuffer::end()
{
    vkEndCommandBuffer(commandBuffer);
}

void CommandBuffer::submit(VkQueue queue, VkFence fence)
{
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    vkQueueSubmit(queue, 1, &submitInfo, fence);
}

void CommandBuffer::submit(VkQueue queue, VkSemaphore drawingCompleteSemaphore, VkSemaphore presentCompleteSemaphore, VkFence fence)
{
	VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = drawingCompleteSemaphore == VK_NULL_HANDLE ? 0 : 1;
	submitInfo.pSignalSemaphores = &drawingCompleteSemaphore;
	submitInfo.waitSemaphoreCount = presentCompleteSemaphore == VK_NULL_HANDLE ? 0 : 1;
	submitInfo.pWaitSemaphores = &presentCompleteSemaphore;
	submitInfo.pWaitDstStageMask = &waitDstStageMask;
	vkQueueSubmit(queue, 1, &submitInfo, fence);
}

void CommandBuffer::beginRenderPass(VkRenderPassBeginInfo* beginInfo)
{
    vkCmdBeginRenderPass(commandBuffer, beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::endRenderPass()
{
    vkCmdEndRenderPass(commandBuffer);
}
