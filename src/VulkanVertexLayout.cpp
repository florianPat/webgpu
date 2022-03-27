#include "VulkanVertexLayout.h"

void VulkanVertexLayout::addAttribute(uint32_t size, Type type)
{
    VkFormat format = getFormatFor(size, type);

	VkVertexInputAttributeDescription vertexInputAttributeDescription = {};
	vertexInputAttributeDescription.location = attributes.size();
	vertexInputAttributeDescription.binding = bindings.size();
	vertexInputAttributeDescription.format = format;
	vertexInputAttributeDescription.offset = stride;
    attributes.push_back(std::move(vertexInputAttributeDescription));

    stride += size * getSizeof(type);
}

void VulkanVertexLayout::set(VkVertexInputRate inputRate)
{
    VkVertexInputBindingDescription binding = {};
	binding.binding = bindings.size();
	binding.stride = stride;
	binding.inputRate = inputRate;

    bindings.push_back(binding);

    stride = 0;
}

VkFormat VulkanVertexLayout::getFormatFor(uint32_t size, Type type)
{
    switch(type)
    {
        case Type::FLOAT:
        {
            switch(size)
            {
                case 4:
                {
                    return VK_FORMAT_R32G32B32A32_SFLOAT;
                }
                case 3:
                {
                    return VK_FORMAT_R32G32B32_SFLOAT;
                }
                case 2:
                {
                    return VK_FORMAT_R32G32_SFLOAT;
                }
                default:
                {
                    InvalidCodePath;
                    break;
                }
            }
            break;
        }
        case Type::UINT32:
        {
            switch (size)
            {
                case 4:
                {
                    return VK_FORMAT_R32G32B32A32_UINT;
                }
                case 3:
                {
                    return VK_FORMAT_R32G32B32_UINT;
                }
                case 2:
                {
                    return VK_FORMAT_R32G32_UINT;
                }
                case 1:
                {
                    return VK_FORMAT_R32_UINT;
                }
            }
        }
        default:
        {
            InvalidCodePath;
            break;
        }
    }

    return VK_FORMAT_UNDEFINED;
}

uint32_t VulkanVertexLayout::getSizeof(VulkanVertexLayout::Type type)
{
    switch(type)
    {
        case Type::FLOAT:
        {
            return sizeof(float);
        }
        case Type::UINT32:
        {
            return sizeof(uint32_t);
        }
        default:
        {
            InvalidCodePath;
            break;
        }
    }

    return 0;
}
