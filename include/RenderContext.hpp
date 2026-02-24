#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

struct RenderContext {
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkRenderPass renderPass;
    VkQueue graphicsQueue;
    VkCommandPool commandPool;
    uint32_t imageCount;
};
