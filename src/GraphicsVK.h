#pragma once

#include "CommandBuffer.h"
#include "VulkanFence.h"
#include "VulkanShader.h"
#include "VulkanDescriptorSetLayout.h"

struct FramebufferDrawStruct
{
	CommandBuffer* currentCommandBuffer = nullptr;
	VulkanFence* currentFence = nullptr;
	VkRenderPassBeginInfo* currentRenderPassBeginInfo = nullptr;
	VkPipeline currentPipeline = (VkPipeline)VK_NULL_HANDLE;
};

class GraphicsVK
{
protected:
	class GraphicsVKIniter& gfxInit;

	FramebufferDrawStruct state;
	// The child classes has to manage these!
	VkPipeline pipeline = VK_NULL_HANDLE;
	VulkanShader shader;
	VulkanDescriptorSetLayout descriptorSetLayout;
public:
	//static constexpr uint32_t id;
	//static constexpr VkBool32 enableAlpha = VK_TRUE;

	GraphicsVK(GraphicsVKIniter& gfxInit) : gfxInit(gfxInit) {}
	GraphicsVK(const GraphicsVK& other) = delete;
	GraphicsVK& operator=(const GraphicsVK& rhs) = delete;
	GraphicsVK(GraphicsVK&& other) = delete;
	GraphicsVK& operator=(GraphicsVK&& rhs) = delete;
	virtual ~GraphicsVK() {};
	virtual void render() {};
	VkPipeline getPipeline() const { return pipeline; }
	const VulkanShader& getShader() const { return shader; }
	const VulkanDescriptorSetLayout& getDescriptorSetLayout() const { return descriptorSetLayout; }
	//void bindOtherFramebuffer(FramebufferDrawStruct&& stateIn);
	//void unbinOtherFramebuffer();
};