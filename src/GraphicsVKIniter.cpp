#include "GraphicsVKIniter.h"
#include "HeapArray.h"
#include <vulkan/vulkan_win32.h>
#include <vulkan/vk_platform.h>
#include "GraphicsVK2D.h"
#include "GraphicsVK3D.h"
#include "VulkanRenderTexture.h"

// TODO: Set this queried!!
uint32_t GraphicsVKIniter::max2dTextureSize = 512;

GraphicsVKIniter::GraphicsVKIniter(uint32_t renderWidth, uint32_t renderHeight) : IGraphics(renderWidth, renderHeight),
    renderers(sizeof(GraphicsVK2D) + sizeof(GraphicsVK3D) + 2 * sizeof(uint32_t))
{
}

bool GraphicsVKIniter::startGfx(HINSTANCE hinstance, HWND hwnd)
{
    if(!createInstance())
        return false;

#ifdef VULKAN_DEBUG
	if (!createDebugExt())
		return false;
#endif

    gpu = getGPU();
    if (gpu == VK_NULL_HANDLE)
    {
        utils::log("Could not get gpu!");
        return false;
    }

    if (!createSurface(hinstance, hwnd))
    {
        utils::log("Could not create surface");
        return false;
    }

    if (!createDevice(gpu, surface))
    {
        utils::log("Could not create device");
        return false;
    }

    vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &queue);
    if (presentQueueFamilyIndex != graphicsQueueFamilyIndex)
    {
        vkGetDeviceQueue(device, presentQueueFamilyIndex, 0, &presentQueue);
    }
    else
    {
        presentQueue = queue;
    }

//    auto vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR) vkGetDeviceProcAddr(device, "vkCreateSwapchainKHR");
//    auto vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR) vkGetDeviceProcAddr(device, "vkDestroySwapchainKHR");
//    auto vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR) vkGetDeviceProcAddr(device, "vkGetSwapchainImagesKHR");
//    auto vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR) vkGetDeviceProcAddr(device, "vkAcquireNextImageKHR");
//    auto vkQueuePresentKHR = (PFN_vkQueuePresentKHR) vkGetDeviceProcAddr(device, "vkQueuePresentKHR");
//    auto vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR) vkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
//    auto vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR) vkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
//    auto vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR) vkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceSurfacePresentModesKHR");

    surfaceFormat = getSurfaceFormat(gpu, surface);
    if (surfaceFormat.format == VK_FORMAT_UNDEFINED)
    {
        utils::log("Could not get surfaceFormat");
        return false;
    }

    VkPresentModeKHR presentMode = getPresentMode(gpu, surface);
    if (presentMode == VK_PRESENT_MODE_MAX_ENUM_KHR)
    {
        utils::log("Could not get presentMode");
        return false;
    }

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VkResult vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surfaceCapabilities);
    if (vkResult != VK_SUCCESS)
    {
        utils::log("Could not get physical device surface capabilities");
        return false;
    }

    uint32_t minImageCount = getNumSwapchainImageBuffers(surfaceCapabilities);
    
	swapchainExtent = getSwapchainExtent(surfaceCapabilities);
	screenWidth = swapchainExtent.width;
	screenHeight = swapchainExtent.height;
    renderWidth = swapchainExtent.width;
    renderHeight = swapchainExtent.height;

    VkSurfaceTransformFlagBitsKHR transformFlag = getSwapchainTransformFlag(surfaceCapabilities);
    VkCompositeAlphaFlagBitsKHR compositeAlphaFlag = getSwapchainAlphaFlag(surfaceCapabilities);

    if (!createSwapchain(minImageCount, surface, surfaceFormat, swapchainExtent, transformFlag, compositeAlphaFlag, presentMode))
    {
        utils::log("Could not create swapchain");
        return false;
    }

    if (!getSwapchainImagesAndViews(surfaceFormat.format))
    {
        utils::log("Could not create swapchain image and views");
        return false;
    }

    if (!createCommandPool())
    {
        utils::log("Could not create command pool");
        return false;
    }

	renderPass = createRenderPass(surfaceFormat.format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    if (!createSwapchainFramebuffers(swapchainExtent))
    {
        utils::log("Could not create swapchain framebuffers");
        return false;
    }

    if (!createPipelineCache())
    {
        utils::log("Could not create pipeline cache");
        return false;
    }

    setClearAndPresentStructs();

    // NOTE: Need to do this because the renderpass`s framebuffer should not get overitten if a renderer gets the next DrawStruct and another one wants to use the old one
    //       after that
    for (uint32_t i = 0; i < arrayCount(renderPassBeginInfos); ++i)
    {
        renderPassBeginInfos[i].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfos[i].pNext = nullptr;
        renderPassBeginInfos[i].renderPass = renderPass;
        renderPassBeginInfos[i].framebuffer = framebuffers[i];
        renderPassBeginInfos[i].clearValueCount = 0;
        renderPassBeginInfos[i].pClearValues = nullptr;
        renderPassBeginInfos[i].renderArea.offset = VkOffset2D{ 0, 0 };
        renderPassBeginInfos[i].renderArea.extent = swapchainExtent;
    }
    for (uint32_t i = 0; i < NUM_BUFFERS; ++i)
    {
        new (&commandBuffers[i]) CommandBuffer(device, commandPool);
    }
    new (&fence) VulkanFence(device);

    VkResult result = vkAcquireNextImageKHR(device, swapchain, INT64_MAX, VK_NULL_HANDLE, fence.fence,
        &currentSwapchainImage);
    assert(result == VK_SUCCESS);
    fence.wait();
    fence.reset();

    setPresentFramebufferTransitionBarriers();

    return true;
}

void GraphicsVKIniter::stopGfx()
{
    vkDeviceWaitIdle(device);

    renderers.~VariableVector();

    VulkanRenderTexture::disposeRenderPass(device);

    fence.~VulkanFence();

	if (pipelineCache != VK_NULL_HANDLE)
	{
		vkDestroyPipelineCache(device, pipelineCache, nullptr);
		pipelineCache = VK_NULL_HANDLE;
	}

    depthTexture.~VulkanTexture();
    for(uint32_t i = 0; i < NUM_BUFFERS; ++i)
    {
		if (framebuffers[i] != VK_NULL_HANDLE)
		{
			vkDestroyFramebuffer(device, framebuffers[i], nullptr);
			framebuffers[i] = VK_NULL_HANDLE;
		}
    }

    if(renderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(device, renderPass, nullptr);
        renderPass = VK_NULL_HANDLE;
    }

    if(commandPool != VK_NULL_HANDLE)
    {
        vkResetCommandPool(device, commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
        vkDestroyCommandPool(device, commandPool, nullptr);
        commandPool = VK_NULL_HANDLE;
        for (uint32_t i = 0; i < NUM_BUFFERS; ++i)
        {
            commandBuffers[i].~CommandBuffer();
        }
    }

	if (swapchain != VK_NULL_HANDLE)
	{
        for (uint32_t i = 0; i < swapchainImageCount; ++i)
        {
            if (swapchainImageViews[i] != VK_NULL_HANDLE)
            {
                vkDestroyImageView(device, swapchainImageViews[i], nullptr);
                swapchainImageViews[i] = VK_NULL_HANDLE;
            }
        }

		vkDestroySwapchainKHR(device, swapchain, nullptr);
		swapchain = VK_NULL_HANDLE;
        for (uint32_t i = 0; i < swapchainImageCount; ++i)
        {
            swapchainImages[i] = VK_NULL_HANDLE;
        }
        swapchainImageCount = 0;
	}
    if(surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(vkInstance, surface, nullptr);
        surface = VK_NULL_HANDLE;
    }

    if(device != nullptr)
    {
        vkDestroyDevice(device, nullptr);
        device = nullptr;
        queue = nullptr;
        presentQueue = nullptr;
    }

	if (debugUtilsMessenger != VK_NULL_HANDLE)
	{
		auto vkDestroyDebugUtilsMessangerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugUtilsMessengerEXT");
		assert(vkDestroyDebugUtilsMessangerEXT);
		vkDestroyDebugUtilsMessangerEXT(vkInstance, debugUtilsMessenger, nullptr);
	}

    if(vkInstance != nullptr)
    {
        vkDestroyInstance(vkInstance, nullptr);
        vkInstance = nullptr;
    }
}

void GraphicsVKIniter::clear()
{
    // TODO: "Optimize" this by just using one render pass and push constants / one large dynamic uniform buffer to store the data and not supply a change into the same one
    // at offset 0
    commandBuffers[currentSwapchainImage].begin();

    commandBuffers[currentSwapchainImage].beginRenderPass(&renderPassBeginInfos[currentSwapchainImage]);

    vkCmdClearAttachments(commandBuffers[currentSwapchainImage].commandBuffer, 2, clearAttachments, 2, clearRects);

    commandBuffers[currentSwapchainImage].endRenderPass();
    commandBuffers[currentSwapchainImage].end();

    commandBuffers[currentSwapchainImage].submit(queue, fence.fence);

    fence.wait();
    fence.reset();
}

void GraphicsVKIniter::render()
{
    for (auto it = renderers.begin(); it != renderers.end();)
    {
        uint32_t size = *((uint32_t*)it);
        it += sizeof(uint32_t);

        ((GraphicsVK*)(it))->render();

        it += size;
    }

    presentInfo.pImageIndices = &currentSwapchainImage;

    vkQueuePresentKHR(presentQueue, &presentInfo);

    VkResult result = vkAcquireNextImageKHR(device, swapchain, INT64_MAX, VK_NULL_HANDLE, fence.fence,
        &currentSwapchainImage);
    assert(result == VK_SUCCESS);
    fence.wait();
    fence.reset();

    assert(currentSwapchainImage == nextSwapchainImage);
    nextSwapchainImage = currentSwapchainImage == 0 ? 1 : 0;
}

FramebufferDrawStruct GraphicsVKIniter::getNextFramebufferStruct(VkPipeline pipeline)
{
    return FramebufferDrawStruct{ &commandBuffers[nextSwapchainImage], &fence, &renderPassBeginInfos[nextSwapchainImage],
        pipeline };
}

FramebufferDrawStruct GraphicsVKIniter::getCurrentFramebufferStruct(VkPipeline pipeline)
{
    return FramebufferDrawStruct{ &commandBuffers[currentSwapchainImage], &fence, &renderPassBeginInfos[currentSwapchainImage],
        pipeline };
}

bool GraphicsVKIniter::createInstance()
{
    uint32_t instanceLayerPropertyCount;
    vkEnumerateInstanceLayerProperties(&instanceLayerPropertyCount, nullptr);
    HeapArray<VkLayerProperties> instanceLayerProperties(instanceLayerPropertyCount, VkLayerProperties{});
    vkEnumerateInstanceLayerProperties(&instanceLayerPropertyCount, instanceLayerProperties.data());

#ifdef VULKAN_DEBUG
    const char* instanceLayerNames[] = { "VK_LAYER_KHRONOS_validation" };
    assert(arrayCount(instanceLayerNames) <= instanceLayerPropertyCount);
    uint32_t instanceLayerNamesArrayCount = arrayCount(instanceLayerNames);
#else
    const char* instanceLayerNames[] = { "No validation for release build because the vulkan sdk is not installed on play computers" };
    uint32_t instanceLayerNamesArrayCount = 0;
#endif

    for(uint32_t i = 0; i < instanceLayerNamesArrayCount; ++i)
    {
        bool found = false;
        for(auto it = instanceLayerProperties.begin(); it != instanceLayerProperties.end(); ++it)
        {
            if(String::createIneffectivlyFrom(instanceLayerNames[i]) == it->layerName)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            utils::log("Did not find an instance layer!");
            return false;
        }
    }

    uint32_t instanceExtensionPropertyCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionPropertyCount, nullptr);
    HeapArray<VkExtensionProperties> instanceExtensionProperties(instanceExtensionPropertyCount, VkExtensionProperties{});
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionPropertyCount, instanceExtensionProperties.data());

#ifdef VULKAN_DEBUG
    const char* instanceExtensionNames[] = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
#else
    const char* instanceExtensionNames[] = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
#endif
    assert(arrayCount(instanceExtensionNames) <= instanceExtensionPropertyCount);

    for(uint32_t i = 0; i < arrayCount(instanceExtensionNames); ++i)
    {
        bool found = false;
        for(auto it = instanceExtensionProperties.begin(); it != instanceExtensionProperties.end(); ++it)
        {
            if(String::createIneffectivlyFrom(instanceExtensionNames[i]) == it->extensionName)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            utils::log("Did not find an instance extension!");
            return false;
        }
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = "3DEngine";
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = "3DEngine";
    appInfo.engineVersion = 1;
    appInfo.apiVersion = VK_API_VERSION_1_1;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = instanceLayerNamesArrayCount;
    createInfo.ppEnabledLayerNames = instanceLayerNames;
    createInfo.enabledExtensionCount = arrayCount(instanceExtensionNames);
    createInfo.ppEnabledExtensionNames = instanceExtensionNames;

    VkResult vkResult = vkCreateInstance(&createInfo, nullptr, &vkInstance);

    bool result = (vkResult == VK_SUCCESS);
    if (!result)
    {
        utils::log("Could not create instance!");
    }
    return result;
}

VkPhysicalDevice GraphicsVKIniter::getGPU() const
{
    uint32_t gpuDeviceCount;
    VkResult vkResult = vkEnumeratePhysicalDevices(vkInstance, &gpuDeviceCount, nullptr);

    if(vkResult != VK_SUCCESS)
        return nullptr;
    if(gpuDeviceCount == 0)
        return nullptr;

    HeapArray<VkPhysicalDevice> gpuList(gpuDeviceCount, nullptr);
    vkResult = vkEnumeratePhysicalDevices(vkInstance, &gpuDeviceCount, gpuList.data());
    if(vkResult != VK_SUCCESS)
        return nullptr;

    //vkGetPhysicalDeviceProperties();

    for (auto it = gpuList.begin(); it != gpuList.end(); ++it)
    {
        VkFormatFeatureFlags featureFlags = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT
            | VK_FORMAT_FEATURE_TRANSFER_DST_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(*it, VK_FORMAT_R8G8B8A8_SRGB, &formatProperties);
        if (formatProperties.optimalTilingFeatures & featureFlags)
        {
            VkPhysicalDeviceProperties deviceProperties = {};
            vkGetPhysicalDeviceProperties(*it, &deviceProperties);
            max2dTextureSize = deviceProperties.limits.maxImageDimension2D;

            return *it;
        }
    }

    return VK_NULL_HANDLE;
}

GraphicsVK* GraphicsVKIniter::getRenderer(uint32_t rendererId)
{
    return (GraphicsVK*)(renderers.begin() + rendererOffsets[rendererId]);
}

uint32_t GraphicsVKIniter::getMemoryTypeIndex(uint32_t memoryRequirementsIndexBits, VkMemoryPropertyFlags memoryPropertyFlags) const
{
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties = {};
    vkGetPhysicalDeviceMemoryProperties(gpu, &physicalDeviceMemoryProperties);
    for(uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; ++i)
    {
		bool isIndexSet = (memoryRequirementsIndexBits >> i) & 1;
        if(isIndexSet && physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags)
        {
			return i;
        }
        //deviceSize size = physicalDeviceMemoryProperties.memoryHeaps[physicalDeviceMemoryProperties.memoryTypes[i].heapIndex].size;
    }

	InvalidCodePath;
	return 0;
}

bool GraphicsVKIniter::createDevice(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
    uint32_t queueFamilyPropertiesCount;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyPropertiesCount, nullptr);
    if(queueFamilyPropertiesCount == 0)
    {
        return false;
    }
    HeapArray<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertiesCount, VkQueueFamilyProperties{});
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyPropertiesCount, queueFamilyProperties.data());
    graphicsQueueFamilyIndex = (uint32_t)-1;
    presentQueueFamilyIndex = (uint32_t)-1;
    for(uint32_t i = 0; i < queueFamilyProperties.size(); ++i)
    {
        VkBool32 hasPresentationSupport;
        VkResult vkResult = vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &hasPresentationSupport);
        if(vkResult != VK_SUCCESS)
            return false;

        if(hasPresentationSupport)
        {
            presentQueueFamilyIndex = i;
        }

        if(queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphicsQueueFamilyIndex = i;
        }

        if(presentQueueFamilyIndex != -1 && graphicsQueueFamilyIndex != -1)
        {
            break;
        }
    }

    if(presentQueueFamilyIndex == -1 && graphicsQueueFamilyIndex == -1)
        return false;

    float queuePriorities[] = { 0.0f };
    VkDeviceQueueCreateInfo queueCreateInfos[2] = {};
    queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[0].queueFamilyIndex = graphicsQueueFamilyIndex;
    queueCreateInfos[0].flags = 0;
    queueCreateInfos[0].pNext = nullptr;
    queueCreateInfos[0].pQueuePriorities = queuePriorities;
    queueCreateInfos[0].queueCount = arrayCount(queuePriorities);
    if (graphicsQueueFamilyIndex != presentQueueFamilyIndex)
    {
        queueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[1].queueFamilyIndex = presentQueueFamilyIndex;
        queueCreateInfos[1].flags = 0;
        queueCreateInfos[1].pNext = nullptr;
        queueCreateInfos[1].pQueuePriorities = queuePriorities;
        queueCreateInfos[1].queueCount = arrayCount(queuePriorities);
    }

    const char* deviceExtensioNames[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = nullptr;
    deviceCreateInfo.flags = 0;
    //NOTE: Device layers are deprecated!
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = nullptr;
    deviceCreateInfo.enabledExtensionCount = arrayCount(deviceExtensioNames);
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensioNames;
    deviceCreateInfo.pEnabledFeatures = nullptr;
    deviceCreateInfo.queueCreateInfoCount = presentQueueFamilyIndex != graphicsQueueFamilyIndex ? 2 : 1;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;

    VkResult vkResult = vkCreateDevice(gpu, &deviceCreateInfo, nullptr, &device);
    return (vkResult == VK_SUCCESS);
}

bool GraphicsVKIniter::createSurface(HINSTANCE hinstance, HWND hwnd)
{
    auto vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR) vkGetInstanceProcAddr(vkInstance, "vkCreateWin32SurfaceKHR");

    VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfo = {};
    win32SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    win32SurfaceCreateInfo.pNext = nullptr;
    win32SurfaceCreateInfo.flags = 0;
	win32SurfaceCreateInfo.hinstance = hinstance;
	win32SurfaceCreateInfo.hwnd = hwnd;
    VkResult vkResult = vkCreateWin32SurfaceKHR(vkInstance, &win32SurfaceCreateInfo, nullptr, &surface);

    return (vkResult == VK_SUCCESS);
}

VkSurfaceFormatKHR GraphicsVKIniter::getSurfaceFormat(VkPhysicalDevice gpu, VkSurfaceKHR surface) const
{
    uint32_t surfaceFormatCount;

    VkResult vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &surfaceFormatCount, nullptr);
    if(vkResult != VK_SUCCESS)
        return { VK_FORMAT_UNDEFINED, VK_COLORSPACE_SRGB_NONLINEAR_KHR };

    assert(surfaceFormatCount != 0);
    HeapArray<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount, VkSurfaceFormatKHR{});

    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &surfaceFormatCount, surfaceFormats.data());
    if(vkResult != VK_SUCCESS)
        return { VK_FORMAT_UNDEFINED, VK_COLORSPACE_SRGB_NONLINEAR_KHR };

    //NOTE: If format is not defined, one can choose one!
    if (surfaceFormatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
    {
        assert(surfaceFormats[0].colorSpace == 0);
        return { VK_FORMAT_B8G8R8A8_SRGB, (VkColorSpaceKHR)0 };
    }
    else
    {
        VkFormat formats[] = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_B8G8R8_SRGB, VK_FORMAT_R8G8B8_SRGB };
        
        for (uint32_t i = 0; i < arrayCount(formats); ++i)
        {
            for (auto it = surfaceFormats.begin(); it != surfaceFormats.end(); ++it)
            {
                //NOTE: 0 is VK_COLOR_SPACE_SRGB_NON_LINEAR_KHR or VK_COLORSPACE_SRGB_NON_LINEAR_KHR
                if (it->format == formats[i] && it->colorSpace == 0)
                    return *it;
            }
        }

        for (auto it = surfaceFormats.begin(); it != surfaceFormats.end(); ++it)
        {
            if (it->colorSpace == 0)
                return *it;
        }
    }

    return { VK_FORMAT_UNDEFINED, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
}

VkPresentModeKHR GraphicsVKIniter::getPresentMode(VkPhysicalDevice gpu, VkSurfaceKHR surface) const
{
    uint32_t presentationModeCount;

    VkResult vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentationModeCount, nullptr);
    if(vkResult != VK_SUCCESS)
        return VK_PRESENT_MODE_MAX_ENUM_KHR;

    assert(presentationModeCount != 0);
    HeapArray<VkPresentModeKHR> presentationModes(presentationModeCount, VK_PRESENT_MODE_MAX_ENUM_KHR);

    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentationModeCount, presentationModes.data());
    if(vkResult != VK_SUCCESS)
        return VK_PRESENT_MODE_MAX_ENUM_KHR;

    for(auto it = presentationModes.begin(); it != presentationModes.end(); ++it)
    {
        if((*it) == VK_PRESENT_MODE_FIFO_KHR)
        {
            //NOTE: This is required to be supported!
            return VK_PRESENT_MODE_FIFO_KHR;
        }
    }

    InvalidCodePath;
    return VK_PRESENT_MODE_FIFO_KHR;
}

bool GraphicsVKIniter::createSwapchain(uint32_t minImageCount, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat,
        VkExtent2D swapchainExtent, VkSurfaceTransformFlagBitsKHR transformFlag, VkCompositeAlphaFlagBitsKHR alphaBit, VkPresentModeKHR presentMode)
{
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.pNext = nullptr;
    swapchainCreateInfo.minImageCount = minImageCount;
    swapchainCreateInfo.flags = 0;
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = swapchainExtent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    swapchainCreateInfo.preTransform = transformFlag;
    swapchainCreateInfo.compositeAlpha = alphaBit;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult vkResult = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);

    return (vkResult == VK_SUCCESS);
}

bool GraphicsVKIniter::getSwapchainImagesAndViews(VkFormat surfaceFormat)
{
    VkResult vkResult = vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr);
    if(vkResult != VK_SUCCESS)
        return false;

    assert(swapchainImageCount == NUM_BUFFERS);
    vkResult = vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages);
    if(vkResult != VK_SUCCESS)
        return false;

    for(uint32_t i = 0; i < swapchainImageCount; ++i)
    {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.pNext = nullptr;
        imageViewCreateInfo.format = surfaceFormat;
        imageViewCreateInfo.flags = 0;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.image = swapchainImages[i];

        vkResult = vkCreateImageView(device, &imageViewCreateInfo, nullptr, &swapchainImageViews[i]);
        if(vkResult != VK_SUCCESS)
            return false;
    }

    return true;
}

uint32_t GraphicsVKIniter::getNumSwapchainImageBuffers(VkSurfaceCapabilitiesKHR surfaceCapabilities) const
{
    //Number of front- and backbuffers
    uint32_t result = NUM_BUFFERS;
    if(result > surfaceCapabilities.maxImageCount)
    {
        result = surfaceCapabilities.maxImageCount;
    }
    assert(result >= surfaceCapabilities.minImageCount);

    return result;
}

VkExtent2D GraphicsVKIniter::getSwapchainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities) const
{
    VkExtent2D result;

    if(surfaceCapabilities.currentExtent.width == 0xFFFFFFFF)
    {
        result = surfaceCapabilities.maxImageExtent;
    }
    else
    {
        result = surfaceCapabilities.currentExtent;
    }

    return result;
}

VkSurfaceTransformFlagBitsKHR GraphicsVKIniter::getSwapchainTransformFlag(VkSurfaceCapabilitiesKHR surfaceCapabilities) const
{
    VkSurfaceTransformFlagBitsKHR result;

    if(surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        result = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    else
        result = surfaceCapabilities.currentTransform;

    return result;
}

VkCompositeAlphaFlagBitsKHR GraphicsVKIniter::getSwapchainAlphaFlag(VkSurfaceCapabilitiesKHR surfaceCapabilities) const
{
    VkCompositeAlphaFlagBitsKHR result = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    //NOTE: One of these is set!
    VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };
    for (uint32_t i = 0; i < sizeof(compositeAlphaFlags); i++)
    {
        if (surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i])
        {
            result = compositeAlphaFlags[i];
            break;
        }
    }

    return result;
}

bool GraphicsVKIniter::createCommandPool()
{
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.pNext = nullptr;
    commandPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT that buffers form this pool will change frequently
    VkResult vkResult = vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool);

    return (vkResult == VK_SUCCESS);
}

VkRenderPass GraphicsVKIniter::createRenderPass(VkFormat surfaceFormat, VkImageLayout finalLayout)
{
    VkAttachmentDescription attachmentDescriptions[2] = {};
    attachmentDescriptions[0].format = surfaceFormat;
    attachmentDescriptions[0].flags = 0;
    attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescriptions[0].initialLayout = finalLayout;
    attachmentDescriptions[0].finalLayout = finalLayout;
    attachmentDescriptions[1].format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    attachmentDescriptions[1].flags = 0;
    attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentReference = {};
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkAttachmentReference depthAttachmentReference = {};
    depthAttachmentReference.attachment = 1;
    depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.flags = 0;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;
    subpass.pDepthStencilAttachment = &depthAttachmentReference;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;
    subpass.pResolveAttachments = nullptr;

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.pNext = nullptr;
    renderPassCreateInfo.flags = 0;
    renderPassCreateInfo.attachmentCount = 2;
    renderPassCreateInfo.pAttachments = attachmentDescriptions;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 0;
    renderPassCreateInfo.pDependencies = nullptr;

	VkRenderPass result;
    VkResult vkResult = vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &result);
	assert(vkResult == VK_SUCCESS);

	return result;
}

bool GraphicsVKIniter::createSwapchainFramebuffers(VkExtent2D swapchainExtent)
{
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = nullptr;
    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.attachmentCount = 2;
    framebufferCreateInfo.width = swapchainExtent.width;
    framebufferCreateInfo.height = swapchainExtent.height;
    framebufferCreateInfo.flags = 0;
    framebufferCreateInfo.layers = 1;

    depthTexture = VulkanTexture::createDepthTexture(swapchainExtent.width, swapchainExtent.height);

    for(uint32_t i = 0; i < NUM_BUFFERS; ++i)
    {
        VkImageView attachments[2] = { swapchainImageViews[i], depthTexture.getTextureView() };
        framebufferCreateInfo.pAttachments = attachments;
        VkResult vkResult = vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffers[i]);
        if(vkResult != VK_SUCCESS)
            return false;
    }

    return true;
}

VkPipelineVertexInputStateCreateInfo GraphicsVKIniter::createPipelineVertexInputState(const VulkanVertexLayout& vertexLayout) const
{
    VkPipelineVertexInputStateCreateInfo result = {};

    result.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    result.pNext = nullptr;
    result.flags = 0;
    result.vertexBindingDescriptionCount = vertexLayout.bindings.size();
    result.pVertexBindingDescriptions = vertexLayout.bindings.data();
    result.vertexAttributeDescriptionCount = vertexLayout.attributes.size();
    result.pVertexAttributeDescriptions = vertexLayout.attributes.data();

    return result;
}

VkPipelineInputAssemblyStateCreateInfo GraphicsVKIniter::createPipelineInputAssemblyState() const
{
    VkPipelineInputAssemblyStateCreateInfo result = {};

    result.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    result.pNext = nullptr;
    result.flags = 0;
    //Can just be used with indexed drawing to seperate geomitry which was batched into one index array
    result.primitiveRestartEnable = VK_FALSE;
    result.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    return result;
}

VkPipelineRasterizationStateCreateInfo GraphicsVKIniter::createPipelineRasterizationState() const
{
    VkPipelineRasterizationStateCreateInfo result = {};

    result.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    result.pNext = nullptr;
    result.flags = 0;
    result.polygonMode = VK_POLYGON_MODE_FILL;
    result.cullMode = VK_CULL_MODE_BACK_BIT;
    result.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    result.depthClampEnable = VK_FALSE;
    result.rasterizerDiscardEnable = VK_FALSE;
    result.depthBiasEnable = VK_FALSE;
    result.depthBiasClamp = 0;
    result.depthBiasConstantFactor = 0;
    result.depthBiasSlopeFactor = 0;
    result.lineWidth = 1.0f;

    return result;
}

VkPipelineColorBlendStateCreateInfo GraphicsVKIniter::createPipelineColorBlendState(VkPipelineColorBlendAttachmentState* attachmentState,
    VkBool32 enable) const
{
    attachmentState->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
    attachmentState->blendEnable = enable;
    attachmentState->colorBlendOp = VK_BLEND_OP_ADD;
    attachmentState->alphaBlendOp = VK_BLEND_OP_ADD;
    attachmentState->srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    attachmentState->srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    attachmentState->dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    attachmentState->dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

    VkPipelineColorBlendStateCreateInfo result = {};

    result.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    result.pNext = nullptr;
    result.flags = 0;
    result.attachmentCount = 1;
    result.pAttachments = attachmentState;
    result.logicOpEnable = VK_FALSE;
    result.blendConstants[0] = 1.0f;
    result.blendConstants[1] = 1.0f;
    result.blendConstants[2] = 1.0f;
    result.blendConstants[3] = 1.0f;

    return result;
}

VkPipelineViewportStateCreateInfo GraphicsVKIniter::createPipelineViewportState(VkViewport* viewport, VkRect2D* scissor,
    const Vector2ui& viewportExtend) const
{
    viewport->x = 0;
    viewport->y = 0;
    viewport->width = (float)viewportExtend.x;
    viewport->height = (float)viewportExtend.y;
    viewport->minDepth = 0.0f;
    viewport->maxDepth = 1.0f;

	scissor->extent = VkExtent2D{viewportExtend.x, viewportExtend.y};
	scissor->offset = VkOffset2D{ 0, 0 };

    VkPipelineViewportStateCreateInfo result = {};

    result.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    result.pNext = nullptr;
    result.flags = 0;
    result.scissorCount = 1;
    result.pScissors = scissor;
    result.viewportCount = 1;
    result.pViewports = viewport;

    return result;
}

VkPipelineMultisampleStateCreateInfo GraphicsVKIniter::createPipelineMultisampleState() const
{
    VkPipelineMultisampleStateCreateInfo result = {};

    result.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    result.pNext = nullptr;
    result.flags = 0;
    result.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    result.sampleShadingEnable = VK_FALSE;
    result.minSampleShading = 0;
    result.pSampleMask = 0;
    result.alphaToCoverageEnable = VK_FALSE;
    result.alphaToOneEnable = VK_FALSE;

    return result;
}

VkPipelineDepthStencilStateCreateInfo GraphicsVKIniter::createPipelineDepthStencilState() const
{
    VkPipelineDepthStencilStateCreateInfo result = {};

    result.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    result.pNext = nullptr;
    result.flags = 0;
    result.depthTestEnable = VK_TRUE;
    result.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    result.depthWriteEnable = VK_TRUE;
    result.depthBoundsTestEnable = VK_FALSE;
    result.stencilTestEnable = VK_FALSE;

    return result;
}

void GraphicsVKIniter::transitionFromPresentToDraw(uint32_t swapchainImageIndex) const
{
    VkImageMemoryBarrier fromPresentToDraw = {};
    fromPresentToDraw.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    fromPresentToDraw.pNext = nullptr;
    fromPresentToDraw.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    fromPresentToDraw.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    fromPresentToDraw.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    fromPresentToDraw.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    fromPresentToDraw.srcQueueFamilyIndex = presentQueueFamilyIndex;
    fromPresentToDraw.dstQueueFamilyIndex = graphicsQueueFamilyIndex;
    fromPresentToDraw.image = swapchainImages[swapchainImageIndex];
    fromPresentToDraw.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    fromPresentToDraw.subresourceRange.baseArrayLayer = 0;
    fromPresentToDraw.subresourceRange.layerCount = 1;
    fromPresentToDraw.subresourceRange.baseMipLevel = 0;
    fromPresentToDraw.subresourceRange.levelCount = 1;

    vkCmdPipelineBarrier(commandBuffers[swapchainImageIndex].commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &fromPresentToDraw);
}

void GraphicsVKIniter::transitionFromDrawToPresent(uint32_t swapchainImageIndex) const
{
    VkImageMemoryBarrier fromDrawToPresent = {};
    fromDrawToPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    fromDrawToPresent.pNext = nullptr;
    fromDrawToPresent.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    fromDrawToPresent.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    fromDrawToPresent.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    fromDrawToPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    fromDrawToPresent.srcQueueFamilyIndex = graphicsQueueFamilyIndex;
    fromDrawToPresent.dstQueueFamilyIndex = presentQueueFamilyIndex;
    fromDrawToPresent.image = swapchainImages[swapchainImageIndex];
    fromDrawToPresent.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    fromDrawToPresent.subresourceRange.baseArrayLayer = 0;
    fromDrawToPresent.subresourceRange.layerCount = 1;
    fromDrawToPresent.subresourceRange.baseMipLevel = 0;
    fromDrawToPresent.subresourceRange.levelCount = 1;

    vkCmdPipelineBarrier(commandBuffers[swapchainImageIndex].commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &fromDrawToPresent);
}

void GraphicsVKIniter::setClearAndPresentStructs()
{
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount = 0;
    presentInfo.pWaitSemaphores = nullptr;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pResults = nullptr;

    for (uint32_t i = 0; i < 4; ++i)
    {
        clearValues[0].color.float32[i] = 0.0f;
    }
    clearValues[1].depthStencil.depth = 1.0f;
    clearValues[1].depthStencil.stencil = 0;

    clearAttachments[0].colorAttachment = 0;
    clearAttachments[0].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clearAttachments[0].clearValue = clearValues[0];
    clearAttachments[1].colorAttachment = 1;
    clearAttachments[1].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    clearAttachments[1].clearValue = clearValues[1];

    clearRects[0].baseArrayLayer = 0;
    clearRects[0].layerCount = 1;
    clearRects[0].rect.offset = VkOffset2D{ 0, 0 };
    clearRects[0].rect.extent = swapchainExtent;
    clearRects[1].baseArrayLayer = 0;
    clearRects[1].layerCount = 1;
    clearRects[1].rect.offset = VkOffset2D{ 0, 0 };
    clearRects[1].rect.extent = swapchainExtent;
}

void GraphicsVKIniter::setPresentFramebufferTransitionBarriers()
{
    VkCommandBuffer vulkanCommandBuffers[NUM_BUFFERS];
    for (uint32_t i = 0; i < NUM_BUFFERS; ++i)
    {
        commandBuffers[i].begin();
        transitionFromPresentToDraw(i);
        transitionFromDrawToPresent(i);
        commandBuffers[i].end();
        vulkanCommandBuffers[i] = commandBuffers[i].commandBuffer;
    }
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.commandBufferCount = 2;
    submitInfo.pCommandBuffers = vulkanCommandBuffers;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    vkQueueSubmit(queue, 1, &submitInfo, fence.fence);
    fence.wait();
    fence.reset();
}

bool GraphicsVKIniter::createPipelineCache()
{
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipelineCacheCreateInfo.pNext = nullptr;
	pipelineCacheCreateInfo.flags = 0;
	pipelineCacheCreateInfo.initialDataSize = 0;
	pipelineCacheCreateInfo.pInitialData = nullptr;

	VkResult vkResult = vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache);
	return (vkResult == VK_SUCCESS);
}

VkPipeline GraphicsVKIniter::createPipeline(const VulkanShader& shader,
                                            const VulkanDescriptorSetLayout& descriptorSetLayout,
                                            VkRenderPass renderPass, const Vector2ui& viewportExtent, VkBool32 enableAlpha, VkPipeline basePipeline)
{
    auto vertexInputState = createPipelineVertexInputState(shader.vertexLayout);
    auto inputAssemblyState = createPipelineInputAssemblyState();
	VkViewport viewport = {};
	VkRect2D scissor = {};
    auto viewportState = createPipelineViewportState(&viewport, &scissor, viewportExtent);
    auto rasterizationState = createPipelineRasterizationState();
    auto multisampleState = createPipelineMultisampleState();
    auto depthStencilState = createPipelineDepthStencilState();
	VkPipelineColorBlendAttachmentState attachmentState = {};
    auto colorBlendState = createPipelineColorBlendState(&attachmentState, enableAlpha);

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.pNext = nullptr;
    graphicsPipelineCreateInfo.flags = basePipeline == VK_NULL_HANDLE ? VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT : VK_PIPELINE_CREATE_DERIVATIVE_BIT;
    graphicsPipelineCreateInfo.stageCount = shader.getStageCount();
    graphicsPipelineCreateInfo.pStages = shader.getStages();
    graphicsPipelineCreateInfo.pVertexInputState = &vertexInputState;
    graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    graphicsPipelineCreateInfo.pTessellationState = nullptr;
    graphicsPipelineCreateInfo.pViewportState = &viewportState;
    graphicsPipelineCreateInfo.pRasterizationState = &rasterizationState;
    graphicsPipelineCreateInfo.pMultisampleState = &multisampleState;
    graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilState;
    graphicsPipelineCreateInfo.pColorBlendState = &colorBlendState;
    graphicsPipelineCreateInfo.pDynamicState = nullptr;
    graphicsPipelineCreateInfo.layout = descriptorSetLayout.pipelineLayout;
    graphicsPipelineCreateInfo.renderPass = renderPass;
    graphicsPipelineCreateInfo.subpass = 0;
    graphicsPipelineCreateInfo.basePipelineHandle = basePipeline;
    graphicsPipelineCreateInfo.basePipelineIndex = -1;

    VkPipeline result;
    VkResult vkResult = vkCreateGraphicsPipelines(device, pipelineCache, 1, &graphicsPipelineCreateInfo, nullptr, &result);
	assert(vkResult == VK_SUCCESS);

    return result;
}

VkPipeline GraphicsVKIniter::createWindowPipeline(const VulkanShader& shader, const VulkanDescriptorSetLayout& descriptorSetLayout, VkBool32 enableAlpha, VkPipeline basePipeline)
{
    return createPipeline(shader, descriptorSetLayout, renderPass,
        Vector2ui{ swapchainExtent.width, swapchainExtent.height }, enableAlpha, basePipeline);
}

VkSampler GraphicsVKIniter::createSampler()
{
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.pNext = nullptr;
	samplerCreateInfo.flags = 0;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
	samplerCreateInfo.anisotropyEnable = VK_FALSE;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.maxAnisotropy = 0.0f;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 32.0f; //NOTE: Just some random value I choosed

	VkSampler result;
	vkCreateSampler(device, &samplerCreateInfo, nullptr, &result);

	return result;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugUtilsMessenger(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                                const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
                                                                void *userData)
{
    const char* typeString = "";
    const char* severityString = "";
    const char* message = callbackData->pMessage;
    bool logBreak = false;

    if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        severityString = "ERROR";
        logBreak = true;
    }
    else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        severityString = "WARNING";
    }
    else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        severityString = "INFO";
    }
    else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
    {
        severityString = "VERBOSE";
    }
    if(messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
    {
        typeString = "Validation";
    }
    else if(messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
    {
        typeString = "Performance";
    }
    else if(messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
    {
        typeString = "General";
    }

    if (logBreak)
    {
        utils::logF("%s %s:", typeString, severityString);
        utils::logBreak(message);
    }
    else
    {
        //utils::logF("%s %s:", typeString, severityString);
        //utils::log(message);
    }

    // Returning false tells the layer not to stop when the event occurs, so
    // they see the same behavior with and without validation layers enabled.
    return VK_FALSE;
}

bool GraphicsVKIniter::createDebugExt()
{
    auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(vkInstance, "vkCreateDebugUtilsMessengerEXT");
    assert(vkCreateDebugUtilsMessengerEXT);

    VkDebugUtilsMessengerCreateInfoEXT  debugUtilsMessengerCreateInfo = {};
    debugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugUtilsMessengerCreateInfo.pNext = nullptr;
    debugUtilsMessengerCreateInfo.flags = 0;
    debugUtilsMessengerCreateInfo.pUserData = nullptr;
    debugUtilsMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugUtilsMessengerCreateInfo.messageSeverity =  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    debugUtilsMessengerCreateInfo.pfnUserCallback = VulkanDebugUtilsMessenger;

    VkResult vkResult = vkCreateDebugUtilsMessengerEXT(vkInstance, &debugUtilsMessengerCreateInfo, nullptr, &debugUtilsMessenger);

    return (vkResult == VK_SUCCESS);
}
