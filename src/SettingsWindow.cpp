#include "SettingsWindow.hpp"
#include "Scene.hpp"
#include "UniformBufferObjects.hpp"
#include "Object.hpp"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"
#include "RenderContext.hpp"
#include <GLFW/glfw3.h>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

SettingsWindow::SettingsWindow(const RenderContext& context) : context(context) {
    init();
}

void SettingsWindow::init() {
    IMGUI_CHECKVERSION();
    ImGuiContext* ctx = ImGui::CreateContext();
    ImGui::SetCurrentContext(ctx);

    io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io->IniFilename = nullptr;


    ImGui_ImplGlfw_InitForVulkan(context.window, true);

    

    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * 11;
    pool_info.poolSizeCount = 11;
    pool_info.pPoolSizes = pool_sizes;
    
    if (vkCreateDescriptorPool(context.device, &pool_info, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool");
    }
    
    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = context.instance;
    init_info.PhysicalDevice = context.physicalDevice;
    init_info.Device = context.device;
    init_info.Queue = context.graphicsQueue;
    init_info.DescriptorPool = descriptorPool;
    init_info.MinImageCount = context.imageCount;
    init_info.ImageCount = context.imageCount;
    init_info.PipelineInfoMain.RenderPass = context.renderPass;
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&init_info);



    // VkCommandBuffer cmd = beginSingleTimeCommands();

    // ImGui_ImplVulkan_CreateFontsTexture(cmd);

    // endSingleTimeCommands(cmd);

    // ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void SettingsWindow::draw(VkCommandBuffer cmd) {
    if (!visible) return;
    if (!scene) return;
    ImGui_ImplVulkan_RenderDrawData(
        ImGui::GetDrawData(),
        cmd
    );
}

void SettingsWindow::update() {
    if (!visible) return;
    if (!scene) return;

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    int width, height;
    glfwGetWindowSize(context.window, &width, &height);
    ImGui::SetNextWindowSize({300, (float)height});
    ImGui::SetNextWindowPos({(float)width-300, 0});

    ImGui::Begin("Settings",nullptr,ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);

    MaterialUBO* materialUbo = scene->selectedObject()->ubo();
    SceneUBO* sceneUBO = scene->ubo();
    
    ImGui::Text("Material:");

    ImGui::ColorEdit3("albedo", glm::value_ptr(materialUbo->albedo));
    ImGui::SliderFloat("roughness", &materialUbo->roughness, 0.0f, 1.0f);
    ImGui::SliderFloat("metallic", &materialUbo->metallic, 0.0f, 1.0f);
    

    ImGui::Text("Light:");

    const char* lightTypes[] = {"directional", "positional"};
    int type = (int)sceneUBO->lightPos.w;
    if (ImGui::Combo("Type", &type, lightTypes, IM_ARRAYSIZE(lightTypes))) {
        sceneUBO->lightPos.w = (float)type;
    }
    ImGui::ColorEdit3("color", glm::value_ptr(sceneUBO->lightColor));
    ImGui::DragFloat3(type?"position":"direction", glm::value_ptr(sceneUBO->lightPos), type?1.0f:0.01f, 0.01f, type?100.0f:1.0f, type?"%.0f":"%.2f");
    
    float textHeight = ImGui::GetTextLineHeightWithSpacing();

    ImGui::SetCursorPosY(height - textHeight - ImGui::GetStyle().WindowPadding.y);

    ImGui::Text("Window size: %dx%d, FPS: %6.2f",width, height, io->Framerate);

    ImGui::End();

    ImGui::Render();
}

void SettingsWindow::setControlledObject(Object* pObject) {
    object = pObject;
}

SettingsWindow::~SettingsWindow() {
    vkDeviceWaitIdle(context.device);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(context.device, descriptorPool, nullptr);
}

void SettingsWindow::toggle() {
    visible = !visible;
}

void SettingsWindow::setScene(Scene* pScene) {
    scene = pScene;
}