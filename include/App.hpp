#pragma once

#include "Camera.hpp"
#include "Config.hpp"
#include "Object.hpp"
#include "RenderContext.hpp"
#include "Scene.hpp"
#include "SettingsWindow.hpp"
#include <memory>
#define GLM_FORCE_RADIANS
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <sys/types.h>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>


class App {
    

private:
    VkInstance instance;
    GLFWwindow* window;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    int graphicsFamily, presentFamily;
    VkSwapchainKHR swapchain;
    uint32_t imageCount;
    VkFormat swapchainFormat;
    VkExtent2D swapchainExtent;
    std::vector<VkImageView> imageViews;
    VkRenderPass renderPass;
    std::vector<VkImage> colorImages;
    std::vector<VkDeviceMemory> colorImageMemories;
    std::vector<VkImageView> colorImageViews;
    std::vector<VkImage> depthImages;
    std::vector<VkDeviceMemory> depthImageMemories;
    std::vector<VkImageView> depthImageViews;
    std::vector<VkFramebuffer> framebuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailable;
    std::vector<VkSemaphore> renderFinished;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    VkDescriptorPool descriptorPool;
    SettingsWindow* settingsWindow = nullptr;
    Scene* scene = nullptr;
    VkSampleCountFlagBits msaaSamples = (VkSampleCountFlagBits) MSAA_SAMPLES;

    size_t currentFrame = 0;


    glm::vec3 bgcolor =  { 0.05f, 0.05f, 0.1f };

    Camera* camera = nullptr;
    bool rotatingCamera = false;

    // mouse
    float lastX = 800 / 2.0f;
    float lastY = 600 / 2.0f;
    bool firstMouse = true;
    float sensitivity = 0.1f;
    GLFWcursor* dragCursor = nullptr;


    void initInstance(const char* appName);

    void initGLFW(const char* appName, int width, int height);

    void initSurface();

    void selectDevice();

    VkSampleCountFlagBits getMaxUsableSampleCount();

    void createSwapchain();

    void createImageViews();

    void createRenderPass();

    void createColorResources();

    void createDepthResources();

    void createImage(uint32_t width,
        uint32_t height,
        VkSampleCountFlagBits samples,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImage& image,
        VkDeviceMemory& imageMemory);

    void createFramebuffers();

    void createCommandPool();

    void createCommandBuffers();

    void recordCommands();
    
    void recordCommandBuffer(VkCommandBuffer cmd, int imageIndex);

    void createSyncObjects();

    void createDescriptorPool();

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);

    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    void recreateSwapchain();

    void drawFrame();


public:
    App(const char* appName, const glm::vec2& windowSize = {1000,600});

    virtual void run();

    RenderContext getRenderContext() const;

    VkExtent2D getSwapchainExtent() const;
    
    VkQueue getGraphicsQueue() const;

    VkCommandPool getCommandPool() const;

    Object* addObject(std::unique_ptr<Object> obj);

    bool framebufferResized;

    void setCamera(Camera* pCamera);

    Camera* getCamera() const;

    // glm::vec2 getWindowSize() const;

    void handleMouseInput(double xpos, double ypos);
    void handleScroll(double xoffset, double yoffset);
    void handleMouseButton(int button, int action);
    void handleKey(int key, int scancode, int action, int mods);

    void addSettingsWindow(SettingsWindow* pSettingsWindow);

    void setScene(Scene* pScene);

    void destroy();
};
