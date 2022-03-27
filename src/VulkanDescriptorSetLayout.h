#pragma once

#include <vulkan/vulkan.h>
#include "HeapArray.h"

class VulkanDescriptorSetLayout
{
    VkDevice device = nullptr;
	VkDescriptorPool pool = VK_NULL_HANDLE;
	VkDescriptorSet set = VK_NULL_HANDLE;
public:
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
	const uint32_t count = 0;
public:
    VulkanDescriptorSetLayout() = default;
    VulkanDescriptorSetLayout(VkDescriptorSetLayoutBinding* layoutBindings, uint32_t count, VkDevice device);
    VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout& other) = delete;
    VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& other) noexcept;
    VulkanDescriptorSetLayout& operator=(const VulkanDescriptorSetLayout& rhs) = delete;
    VulkanDescriptorSetLayout& operator=(VulkanDescriptorSetLayout&& rhs) noexcept;
    ~VulkanDescriptorSetLayout();
	void bind(VkCommandBuffer commandBuffer);
	void updateUniformBuffer(uint32_t binding, VkDescriptorBufferInfo* descriptorBufferInfo);
	void updateSamplerImage(uint32_t binding, VkImageView imageView, VkSampler sampler);
	VkDescriptorPool createDescriptorPool(VkDescriptorPoolSize* descriptorPoolSizes, uint32_t count);
	VkDescriptorSet createDescriptorSets(VkDescriptorSetLayout setLayout);
private:
	void createPipelineLayout();
};
