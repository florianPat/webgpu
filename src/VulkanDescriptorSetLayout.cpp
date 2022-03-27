#include "VulkanDescriptorSetLayout.h"

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
{
	if (pipelineLayout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		pipelineLayout = VK_NULL_HANDLE;
	}

	if (pool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(device, pool, nullptr);
		pool = VK_NULL_HANDLE;
		set = VK_NULL_HANDLE;
	}

	if (layout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(device, layout, nullptr);
		layout = VK_NULL_HANDLE;
	}

    device = nullptr;
}

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VkDescriptorSetLayoutBinding* layoutBindings,
                                                     uint32_t count, VkDevice device)
                                                     : device(device), count(count)
{
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.pNext = nullptr;
    descriptorSetLayoutCreateInfo.flags = 0;
    descriptorSetLayoutCreateInfo.bindingCount = count;
	descriptorSetLayoutCreateInfo.pBindings = layoutBindings;
	vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &layout);

	HeapArray<VkDescriptorPoolSize> descriptorPoolSizes(count);
	VkDescriptorPoolSize poolSize = {};
	for (uint32_t i = 0; i < count; ++i)
	{
		poolSize.descriptorCount = layoutBindings[i].descriptorCount;
		poolSize.type = layoutBindings[i].descriptorType;
		descriptorPoolSizes.push_back(poolSize);
	}

	pool = createDescriptorPool(descriptorPoolSizes.data(), count);

	set = createDescriptorSets(layout);

	createPipelineLayout();
}

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& other) noexcept
	: device(std::exchange(other.device, nullptr)),
	pool(std::exchange(other.pool, nullptr)),
    set(std::exchange(other.set, nullptr)),
	pipelineLayout(std::exchange(other.pipelineLayout, (VkPipelineLayout)VK_NULL_HANDLE)),
	layout(std::exchange(other.layout, (VkDescriptorSetLayout)VK_NULL_HANDLE)),
	count(other.count)
{
}

VulkanDescriptorSetLayout& VulkanDescriptorSetLayout::operator=(VulkanDescriptorSetLayout&& rhs) noexcept
{
    this->~VulkanDescriptorSetLayout();

    new (this) VulkanDescriptorSetLayout(std::move(rhs));

    return *this;
}

void VulkanDescriptorSetLayout::bind(VkCommandBuffer commandBuffer)
{
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipelineLayout, 0, 1, &set, 0, nullptr);
}

VkDescriptorPool VulkanDescriptorSetLayout::createDescriptorPool(VkDescriptorPoolSize* descriptorPoolSizes, uint32_t count)
{
	VkDescriptorPool result;

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext = nullptr;
	descriptorPoolCreateInfo.flags = 0;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount = count;
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes;

	vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &result);

	return result;
}

VkDescriptorSet VulkanDescriptorSetLayout::createDescriptorSets(VkDescriptorSetLayout setLayout)
{
	VkDescriptorSet result;

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.pNext = nullptr;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &setLayout;
	descriptorSetAllocateInfo.descriptorPool = pool;

	vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &result);

	return result;
}

void VulkanDescriptorSetLayout::updateUniformBuffer(uint32_t binding, VkDescriptorBufferInfo* descriptorBufferInfo)
{
	VkWriteDescriptorSet writeDescriptorSet = {};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.pNext = nullptr;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet.dstArrayElement = 0;
	writeDescriptorSet.dstBinding = binding;
	writeDescriptorSet.dstSet = set;
	writeDescriptorSet.pBufferInfo = descriptorBufferInfo;
	writeDescriptorSet.pImageInfo = nullptr;
	writeDescriptorSet.pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
}

void VulkanDescriptorSetLayout::updateSamplerImage(uint32_t binding, VkImageView imageView, VkSampler sampler)
{
	VkDescriptorImageInfo descriptorImageInfo = {};
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	descriptorImageInfo.imageView = imageView;
	descriptorImageInfo.sampler = sampler;

	VkWriteDescriptorSet writeDescriptorSet = {};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.pNext = nullptr;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSet.dstArrayElement = 0;
	writeDescriptorSet.dstBinding = binding;
	writeDescriptorSet.dstSet = set;
	writeDescriptorSet.pBufferInfo = nullptr;
	writeDescriptorSet.pImageInfo = &descriptorImageInfo;
	writeDescriptorSet.pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
}

void VulkanDescriptorSetLayout::createPipelineLayout()
{
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = nullptr;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &layout;

	vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
}
