#pragma once

#include "Camera.hpp"
#include "Object.hpp"
#include "RenderContext.hpp"
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
    std::vector<VkFramebuffer> framebuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailable;
    std::vector<VkSemaphore> renderFinished;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    VkDescriptorPool descriptorPool;

    size_t currentFrame = 0;

    std::vector<Object> objects;

    glm::vec3 bgcolor =  { 0.05f, 0.05f, 0.1f };

    Camera* camera = nullptr;
    bool rotatingCamera = false;

    // mouse
    float lastX = 800 / 2.0f;
    float lastY = 600 / 2.0f;
    bool firstMouse = true;
    float sensitivity = 0.1f;


    void initInstance(const char* appName);

    void initGLFW(const char* appName, int width, int height);

    void initSurface();

    void selectDevice();

    void createSwapchain();

    void createImageViews();

    void createRenderPass();

    void createFramebuffers();

    void createCommandPool();

    void createCommandBuffers();

    void recordCommands();

    void createSyncObjects();

    void createDescriptorPool();

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);

    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    void recreateSwapchain();


public:
    App(const char* appName, const glm::vec2& windowSize);

    virtual void run();

    RenderContext getRenderContext() const;

    VkExtent2D getSwapchainExtent() const;
    
    VkQueue getGraphicsQueue() const;

    VkCommandPool getCommandPool() const;

    void addObject(const Object& object);

    bool framebufferResized;

    void setCamera(Camera* pCamera) {
        camera = pCamera;
    }

    Camera* getCamera() const {
        return camera;
    }

    void handleMouseInput(double xpos, double ypos);
    void handleScroll(double xoffset, double yoffset);
    void handleMouseButton(int button, int action);

    ~App();
};
