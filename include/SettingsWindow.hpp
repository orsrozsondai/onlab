#pragma once

#include "Object.hpp"
#include "RenderContext.hpp"
#include "Scene.hpp"
#include "imgui.h"
#include <vulkan/vulkan_core.h>


class SettingsWindow {
private:
    RenderContext context;
    VkDescriptorPool descriptorPool;
    ImGuiIO* io;
    Object* object;
    Scene* scene = nullptr;
    bool visible = true;


    void init();
    void initStyle();

public:
    SettingsWindow(const RenderContext& context);
    void draw(VkCommandBuffer cmd);
    void update();
    void setControlledObject(Object* pObject);
    void toggle();
    void setScene(Scene* pScene);
    ~SettingsWindow();
};