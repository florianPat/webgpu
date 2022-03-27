#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>
#include "Vector.h"

struct VulkanVertexLayout
{
    Vector<VkVertexInputBindingDescription> bindings;
    Vector<VkVertexInputAttributeDescription> attributes;
    uint32_t stride = 0;
public:
    enum class Type
    {
        FLOAT,
        UINT32,
    };
private:
    VkFormat getFormatFor(uint32_t size, Type type);
    uint32_t getSizeof(Type type);
public:
    VulkanVertexLayout() = default;
    void addAttribute(uint32_t size, Type type);
    void set(VkVertexInputRate inputRate);
};
