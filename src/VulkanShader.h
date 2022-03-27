#pragma once

#include <vulkan/vulkan.h>
#include "String.h"
#include "VulkanVertexLayout.h"

class VulkanShader
{
    VkDevice device = nullptr;
    uint32_t shaderCount = 0;
    VkShaderModule shaderModule[2] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
    VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo[2] = {};
public:
	VulkanVertexLayout vertexLayout;
public:
    VulkanShader() = default;
    //NOTE: Shader needs to be in compiled form!
    VulkanShader(const LongString& filename, VkDevice device);
    VulkanShader(const VulkanShader& other) = delete;
    VulkanShader(VulkanShader&& other) noexcept;
    VulkanShader& operator=(const VulkanShader& rhs) = delete;
    VulkanShader& operator=(VulkanShader&& rhs) noexcept;
    ~VulkanShader();
    uint32_t getStageCount() const;
    const VkPipelineShaderStageCreateInfo* getStages() const;
};
