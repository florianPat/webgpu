#pragma once

#include "IGraphics.h"
#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>
#include <vulkan/vulkan.h>
#include "CommandBuffer.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanVertexLayout.h"
#include "VulkanShader.h"
#include "VulkanFence.h"
#include "VariableVector.h"
#include "GraphicsVK.h"
#include "VulkanTexture.h"

class GraphicsVKIniter : public IGraphics
{
public:
	VkDevice device = nullptr;
	VkCommandPool commandPool = VK_NULL_HANDLE;
	uint32_t graphicsQueueFamilyIndex = (uint32_t)-1;
    uint32_t presentQueueFamilyIndex = (uint32_t)-1;
	VkQueue queue = nullptr;
    VkQueue presentQueue = nullptr;
	VkPipelineCache pipelineCache = VK_NULL_HANDLE;
    static uint32_t max2dTextureSize;
protected:
    VkInstance vkInstance = nullptr;
    VkPhysicalDevice gpu = nullptr;
    VkDebugUtilsMessengerEXT debugUtilsMessenger = VK_NULL_HANDLE;
    static constexpr uint32_t NUM_BUFFERS = 2;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    uint32_t swapchainImageCount = 0;
	VkImage swapchainImages[NUM_BUFFERS] = { VK_NULL_HANDLE };
    VkImageView swapchainImageViews[NUM_BUFFERS] = { VK_NULL_HANDLE };
    VulkanTexture depthTexture;
    VkFramebuffer framebuffers[NUM_BUFFERS] = { VK_NULL_HANDLE };
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkExtent2D swapchainExtent;
    VkSurfaceFormatKHR surfaceFormat = { VK_FORMAT_UNDEFINED, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
    VkPresentInfoKHR presentInfo = {};
    uint32_t currentSwapchainImage = 0;
    uint32_t nextSwapchainImage = 1;
    CommandBuffer commandBuffers[NUM_BUFFERS];
    VulkanFence fence;
    VkRenderPassBeginInfo renderPassBeginInfos[NUM_BUFFERS] = {};
    VkClearValue clearValues[2] = {};
    VkClearAttachment clearAttachments[2] = {};
    VkClearRect clearRects[2] = {};
    uint32_t rendererOffsets[4] = { 0 };
    VariableVector<GraphicsVK> renderers;
public:
    GraphicsVKIniter(uint32_t renderWidth, uint32_t renderHeight);

    bool startGfx(HINSTANCE hinstance, HWND hwnd);
    void stopGfx();
    void clear();
    void render();
    FramebufferDrawStruct getNextFramebufferStruct(VkPipeline pipeline);
    FramebufferDrawStruct getCurrentFramebufferStruct(VkPipeline pipeline);
    template <typename T, typename... Args>
    void addRenderer(Args&&... args);
    template <typename T>
    T* getRenderer();
    GraphicsVK* getRenderer(uint32_t rendererId);

	uint32_t getMemoryTypeIndex(uint32_t memoryRequirementsIndexBits, VkMemoryPropertyFlags memoryPropertyFlags) const;
	VkRenderPass createRenderPass(VkFormat surfaceFormat, VkImageLayout finalLayout);
	VkPipeline createPipeline(const VulkanShader& shader, const VulkanDescriptorSetLayout& descriptorSetLayout,
		VkRenderPass renderPass, const Vector2ui& viewportExtent, VkBool32 enableAlpha, VkPipeline basePipeline);
    VkPipeline createWindowPipeline(const VulkanShader& shader, const VulkanDescriptorSetLayout& descriptorSetLayout,
        VkBool32 enableAlpha, VkPipeline basePipeline);
	VkSampler createSampler();
    VkFormat getSurfaceFormat() const { return surfaceFormat.format; }
private:
    bool createInstance();
	bool createDebugExt();
    VkPhysicalDevice getGPU() const;
    bool createDevice(VkPhysicalDevice gpu, VkSurfaceKHR surface);
    bool createSurface(HINSTANCE hinstance, HWND hwnd);
    VkSurfaceFormatKHR getSurfaceFormat(VkPhysicalDevice gpu, VkSurfaceKHR surface) const;
    VkPresentModeKHR getPresentMode(VkPhysicalDevice gpu, VkSurfaceKHR surface) const;
    uint32_t getNumSwapchainImageBuffers(VkSurfaceCapabilitiesKHR surfaceCapabilities) const;
    VkExtent2D getSwapchainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities) const;
    VkSurfaceTransformFlagBitsKHR getSwapchainTransformFlag(VkSurfaceCapabilitiesKHR surfaceCapabilities) const;
    VkCompositeAlphaFlagBitsKHR getSwapchainAlphaFlag(VkSurfaceCapabilitiesKHR surfaceCapabilities) const;
    bool createSwapchain(uint32_t minImageCount, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat, VkExtent2D swapchainExtent,
            VkSurfaceTransformFlagBitsKHR transformFlag, VkCompositeAlphaFlagBitsKHR alphaBit, VkPresentModeKHR presentMode);
    bool getSwapchainImagesAndViews(VkFormat surfaceFormat);
    bool createSwapchainFramebuffers(VkExtent2D swapchainExtent);

    bool createCommandPool();
    bool createPipelineCache();
    void setClearAndPresentStructs();
    void setPresentFramebufferTransitionBarriers();
protected:
    VkPipelineVertexInputStateCreateInfo createPipelineVertexInputState(const VulkanVertexLayout& vertexLayout) const;
    VkPipelineInputAssemblyStateCreateInfo createPipelineInputAssemblyState() const;
    VkPipelineRasterizationStateCreateInfo createPipelineRasterizationState() const;
    VkPipelineColorBlendStateCreateInfo createPipelineColorBlendState(VkPipelineColorBlendAttachmentState* attachmentState,
        VkBool32 enable) const;
    VkPipelineViewportStateCreateInfo createPipelineViewportState(VkViewport* viewport, VkRect2D* scissor,
        const Vector2ui& viewportExtend) const;
    VkPipelineMultisampleStateCreateInfo createPipelineMultisampleState() const;
    VkPipelineDepthStencilStateCreateInfo createPipelineDepthStencilState() const;
    void transitionFromPresentToDraw(uint32_t commandBufferIndex) const;
    void transitionFromDrawToPresent(uint32_t commandBufferIndex) const;
};

template<typename T, typename... Args>
inline void GraphicsVKIniter::addRenderer(Args&&... args)
{
    rendererOffsets[T::id] = renderers.getOffsetToEnd() + sizeof(uint32_t);
    renderers.push_back<T>(*this, std::forward<Args>(args)...);
    T* renderer = (T*)(renderers.begin() + rendererOffsets[T::id]);
    renderer->unbindOtherFramebuffer();
}

template<typename T>
inline T* GraphicsVKIniter::getRenderer()
{
    return (T*)(renderers.begin() + rendererOffsets[T::id]);
}