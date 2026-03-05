#pragma once

#include "RenderContext.hpp"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include <vulkan/vulkan_core.h>


class SettingsWindow {
private:
    RenderContext context;
    VkDescriptorPool descriptorPool;
    ImGuiIO* io;


    void init();

public:
    SettingsWindow(const RenderContext& context);
    void draw(VkCommandBuffer cmd);
    void update();
    ~SettingsWindow();
};