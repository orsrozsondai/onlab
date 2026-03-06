#pragma once

#include "Object.hpp"
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
    Object* object;
    bool visible = true;


    void init();

public:
    SettingsWindow(const RenderContext& context);
    void draw(VkCommandBuffer cmd);
    void update();
    void setControlledObject(Object* pObject);
    void toggle();
    ~SettingsWindow();
};