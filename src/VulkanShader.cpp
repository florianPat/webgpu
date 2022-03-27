#include "VulkanShader.h"
#include "Ifstream.h"

VulkanShader::VulkanShader(const LongString& filename, VkDevice device) : device(device), shaderCount(2)
{
    Ifstream vertexShader(filename + ".vert.sprv");
    assert(vertexShader);
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.pNext = nullptr;
    shaderModuleCreateInfo.flags = 0;
    shaderModuleCreateInfo.codeSize = (size_t) vertexShader.getSize();
    shaderModuleCreateInfo.pCode = (const uint32_t*) vertexShader.getFullData();
    vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule[0]);

    Ifstream fragmentShader(filename + ".frag.sprv");
    assert(fragmentShader);
    shaderModuleCreateInfo.codeSize = (size_t) fragmentShader.getSize();
    shaderModuleCreateInfo.pCode = (const uint32_t*) fragmentShader.getFullData();
    vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule[1]);

    pipelineShaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineShaderStageCreateInfo[0].pNext = nullptr;
    pipelineShaderStageCreateInfo[0].flags = 0;
    pipelineShaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    pipelineShaderStageCreateInfo[0].module = shaderModule[0];
    pipelineShaderStageCreateInfo[0].pName = "main";
    pipelineShaderStageCreateInfo[0].pSpecializationInfo = nullptr;

    pipelineShaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineShaderStageCreateInfo[1].pNext = nullptr;
    pipelineShaderStageCreateInfo[1].flags = 0;
    pipelineShaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    pipelineShaderStageCreateInfo[1].module = shaderModule[1];
    pipelineShaderStageCreateInfo[1].pName = "main";
    pipelineShaderStageCreateInfo[1].pSpecializationInfo = nullptr;
}

VulkanShader::~VulkanShader()
{
	for (uint32_t i = 0; i < shaderCount; ++i)
	{
		if (shaderModule[i] != VK_NULL_HANDLE)
		{
			vkDestroyShaderModule(device, shaderModule[i], nullptr);
			shaderModule[i] = VK_NULL_HANDLE;
		}
	}

    vertexLayout.~VulkanVertexLayout();

	device = nullptr;
	shaderCount = 0;
}

uint32_t VulkanShader::getStageCount() const
{
    return shaderCount;
}

const VkPipelineShaderStageCreateInfo* VulkanShader::getStages() const
{
    return pipelineShaderStageCreateInfo;
}

VulkanShader::VulkanShader(VulkanShader&& other) noexcept : device(std::exchange(other.device, nullptr)),
    shaderCount(std::exchange(other.shaderCount, 0)), vertexLayout(std::move(other.vertexLayout))
{
    for(uint32_t i = 0; i < 2; ++i)
    {
        shaderModule[i] = std::exchange(other.shaderModule[i], (VkShaderModule)VK_NULL_HANDLE);
        pipelineShaderStageCreateInfo[i] = std::exchange(other.pipelineShaderStageCreateInfo[i], {});
    }
}

VulkanShader& VulkanShader::operator=(VulkanShader&& rhs) noexcept
{
    this->~VulkanShader();

    new (this) VulkanShader(std::move(rhs));

    return *this;
}
