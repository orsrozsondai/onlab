#pragma once
#include "backends/imgui_impl_glfw.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

struct RenderContext {
    VkInstance instance;
    GLFWwindow* window;
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkRenderPass renderPass;
    VkQueue graphicsQueue;
    VkCommandPool commandPool;
    uint32_t imageCount;
    VkDescriptorPool descriptorPool;
};
