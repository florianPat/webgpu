#include <utility>
#include "VulkanBuffer.h"
#include "Utils.h"
#include <new>
#include "GraphicsVKIniter.h"

VulkanBuffer::VulkanBuffer(uint64_t size, VkBufferUsageFlags usageFlag, const GraphicsVKIniter& gfx) : device(gfx.device), size(size)
{
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.pNext = nullptr;
    bufferCreateInfo.queueFamilyIndexCount = 0;
    bufferCreateInfo.pQueueFamilyIndices = nullptr;
    bufferCreateInfo.flags = 0;
    bufferCreateInfo.size = size;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferCreateInfo.usage = usageFlag;
    vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer);

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);
	assert(memoryRequirements.memoryTypeBits != 0);
	uint32_t memoryTypeIndex = gfx.getMemoryTypeIndex(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    assert(size <= memoryRequirements.size);

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = nullptr;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

    vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &memory);

    vkBindBufferMemory(device, buffer, memory, 0);
}

VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept : device(std::exchange(other.device, nullptr)),
	size(std::exchange(other.size, 0)),
    buffer(std::exchange(other.buffer, (VkBuffer)VK_NULL_HANDLE)), memory(std::exchange(other.memory, (VkDeviceMemory)VK_NULL_HANDLE))
{
}

VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& rhs) noexcept
{
    this->~VulkanBuffer();

    new (this) VulkanBuffer(std::move(rhs));

    return *this;
}

VulkanBuffer::~VulkanBuffer()
{
    if(buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, buffer, nullptr);
        buffer = VK_NULL_HANDLE;
        vkFreeMemory(device, memory, nullptr);
        memory = VK_NULL_HANDLE;
        device = nullptr;
    }
}

VulkanBuffer::operator bool() const
{
    return (buffer != VK_NULL_HANDLE);
}

void VulkanBuffer::bindVertexBuffer(VkCommandBuffer commandBuffer, uint32_t bindingIndex)
{
	VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, bindingIndex, 1, &buffer, &offset);
}

void VulkanBuffer::bindIndexBuffer(VkCommandBuffer commandBuffer)
{
    vkCmdBindIndexBuffer(commandBuffer, buffer, 0, VK_INDEX_TYPE_UINT16);
}

void VulkanBuffer::cmdSubData(const CommandBuffer& commandBuffer, uint32_t offset, uint32_t size, const void* data)
{
	vkCmdUpdateBuffer(commandBuffer.commandBuffer, buffer, offset, size, data);
}

uint64_t VulkanBuffer::getSize() const
{
    return size;
}

uint8_t* VulkanBuffer::map(uint32_t offset, uint64_t size)
{
    uint8_t* mappedData = nullptr;
    vkMapMemory(device, memory, offset, size, 0, (void**)&mappedData);
    return mappedData;
}

void VulkanBuffer::unmap(uint32_t offset, uint64_t size)
{
    VkMappedMemoryRange mappedMemoryRange = {};
	mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedMemoryRange.pNext = nullptr;
	mappedMemoryRange.memory = memory;
	mappedMemoryRange.offset = offset;
	mappedMemoryRange.size = size;
	vkFlushMappedMemoryRanges(device, 1, &mappedMemoryRange);
    vkUnmapMemory(device, memory);
}
