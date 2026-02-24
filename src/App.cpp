#include "App.hpp"
#include "Object.hpp"
#include "Pipeline.hpp"
#include "UniformBufferObject.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>


void App::initInstance(const char* appName) {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = appName;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    uint32_t count;
    const char** extensions = glfwGetRequiredInstanceExtensions(&count);
    const char* layers[] = { "VK_LAYER_KHRONOS_validation" };

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = count;
    createInfo.ppEnabledExtensionNames = extensions;
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = layers;


    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }

}
void App::initGLFW(const char* appName, int width, int height) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(width, height, appName, nullptr, nullptr);
}

void App::initSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

void App::selectDevice() {
    physicalDevice = VK_NULL_HANDLE;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("no GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    physicalDevice = devices[0];

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice,
        &queueFamilyCount,
        nullptr
    );

    std::vector<VkQueueFamilyProperties> families(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice,
        &queueFamilyCount,
        families.data()
    );

    graphicsFamily = -1;
    presentFamily = -1;

    for (uint32_t i = 0; i < queueFamilyCount; i++) {

        bool graphics = families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;

        VkBool32 present = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(
            physicalDevice,
            i,
            surface,
            &present
        );

        // ⭐ BEST CASE: one family does both
        if (graphics && present) {
            graphicsFamily = i;
            presentFamily  = i;
            break;
        }

        if (graphics && graphicsFamily == -1)
            graphicsFamily = i;

        if (present && presentFamily == -1)
            presentFamily = i;
    }

    if (graphicsFamily == -1 || presentFamily == -1) {
        throw std::runtime_error("required queue families not found!");
    }
    std::vector<uint32_t> uniqueFamilies;

    uniqueFamilies.push_back(graphicsFamily);
    
    if (presentFamily != graphicsFamily) {
        uniqueFamilies.push_back(presentFamily);
    }

    // create logical device
    float priority = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    

    for (uint32_t family : uniqueFamilies) {
        VkDeviceQueueCreateInfo q{};
        q.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        q.queueFamilyIndex = family;
        q.queueCount = 1;
        q.pQueuePriorities = &priority;

        queueInfos.push_back(q);
    }
    const char* deviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = queueInfos.size();
    createInfo.pQueueCreateInfos = queueInfos.data();

    createInfo.enabledExtensionCount = 1;
    createInfo.ppEnabledExtensionNames = deviceExtensions;


    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }
    // get queues

    vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(device, presentFamily, 0, &presentQueue);

    
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physicalDevice, &props);

    std::cout << "Selected GPU: " << props.deviceName << std::endl;

}

void App::createSwapchain() {
    struct SwapchainSupport {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    SwapchainSupport support;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physicalDevice, surface, &support.capabilities
    );

    uint32_t count;

    vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice, surface, &count, nullptr
    );
    support.formats.resize(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice, surface, &count, support.formats.data()
    );

    vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice, surface, &count, nullptr
    );
    support.presentModes.resize(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice, surface, &count, support.presentModes.data()
    );

    // choose format
    VkSurfaceFormatKHR format = support.formats[0];
    for (const auto& f : support.formats) {
        if (f.format == VK_FORMAT_B8G8R8A8_SRGB &&
            f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            format = f;
            break;
        }
    }
    swapchainFormat = format.format;
    // choose present mode
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; // vsync
    // for (auto m : support.presentModes) {
    //     if (m == VK_PRESENT_MODE_MAILBOX_KHR) {
    //         presentMode = m; // triple buffering
    //         break;
    //     }

    // }

    // choose extent
    VkExtent2D extent;
    if (support.capabilities.currentExtent.width != UINT32_MAX) {
        extent = support.capabilities.currentExtent;
    }
    else {

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);

        extent = {
            (uint32_t)w,
            (uint32_t)h
        };

        extent.width = std::clamp(
            extent.width,
            support.capabilities.minImageExtent.width,
            support.capabilities.maxImageExtent.width
        );
        extent.height = std::clamp(
            extent.height,
            support.capabilities.minImageExtent.height,
            support.capabilities.maxImageExtent.height
        );
    }
    swapchainExtent = extent;


    imageCount = support.capabilities.minImageCount + 1;

    if (support.capabilities.maxImageCount > 0 &&
        imageCount > support.capabilities.maxImageCount) {
        imageCount = support.capabilities.maxImageCount;
    }    


    VkSwapchainCreateInfoKHR info{};
    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.surface = surface;

    info.minImageCount = imageCount;
    info.imageFormat = format.format;
    info.imageColorSpace = format.colorSpace;
    info.imageExtent = extent;
    info.imageArrayLayers = 1;
    info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilies[] = {
        (uint32_t)graphicsFamily,
        (uint32_t)presentFamily
    };

    if (graphicsFamily != presentFamily) {
        info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        info.queueFamilyIndexCount = 2;
        info.pQueueFamilyIndices = queueFamilies;
    } else {
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    info.preTransform = support.capabilities.currentTransform;
    info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    info.presentMode = presentMode;
    info.clipped = VK_TRUE;
    info.oldSwapchain = VK_NULL_HANDLE;

    std::cout << "device: " << device << "\n";
    std::cout << "surface: " << surface << "\n";
    std::cout << "imageCount: " << info.minImageCount << "\n";
    std::cout << "format: " << info.imageFormat << "\n";
    std::cout << "extent: " << info.imageExtent.width 
          << "x" << info.imageExtent.height << "\n";


    if (vkCreateSwapchainKHR(device, &info, nullptr, &swapchain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swapchain!");
    }
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);

}

void App::createImageViews() {

    imageViews = std::vector<VkImageView>(imageCount);
    std::vector<VkImage> images(imageCount);

    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data());
    for (size_t i = 0; i < images.size(); i++) {
        VkImageViewCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

        info.image = images[i];
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.format = swapchainFormat;

        info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &info, nullptr, &imageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image view!");
        }
    }

}

void App::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchainFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;   // clear every frame
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // keep for presenting

    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorRef{};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;

    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    info.attachmentCount = 1;
    info.pAttachments = &colorAttachment;

    info.subpassCount = 1;
    info.pSubpasses = &subpass;

    info.dependencyCount = 1;
    info.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &info, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }

}

void App::createFramebuffers() {
    framebuffers = std::vector<VkFramebuffer>(imageViews.size());
    for (size_t i = 0; i < imageViews.size(); i++) {
        VkImageView attachments[] = {
            imageViews[i]
        };

        VkFramebufferCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

        info.renderPass = renderPass;
        info.attachmentCount = 1;
        info.pAttachments = attachments;

        info.width  = swapchainExtent.width;
        info.height = swapchainExtent.height;
        info.layers = 1;

        if (vkCreateFramebuffer(device, &info, nullptr, &framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}


void App::createCommandPool() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = graphicsFamily;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }

}

void App::createCommandBuffers() {
    commandBuffers = std::vector<VkCommandBuffer>(framebuffers.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}
void App::recordCommands() {

    for (size_t i = 0; i < commandBuffers.size(); i++) {

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

        VkClearValue clearColor = { bgcolor.x, bgcolor.y, bgcolor.z, 1.0};

        VkRenderPassBeginInfo renderInfo{};
        renderInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderInfo.renderPass = renderPass;
        renderInfo.framebuffer = framebuffers[i];

        renderInfo.renderArea.offset = {0, 0};
        renderInfo.renderArea.extent = swapchainExtent;

        renderInfo.clearValueCount = 1;
        renderInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(
            commandBuffers[i],
            &renderInfo,
            VK_SUBPASS_CONTENTS_INLINE
        );

        for (Object& object : objects) {

            object.getPipeline()->bind(commandBuffers[i]);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width  = (float)swapchainExtent.width;
            viewport.height = (float)swapchainExtent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = swapchainExtent;

            vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

            object.draw(commandBuffers[i], i);

        }


        // vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffers[i]);

        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

}

void App::createSyncObjects() {

    imageAvailable = std::vector<VkSemaphore>(imageCount);
    renderFinished = std::vector<VkSemaphore>(imageCount);
    inFlightFences = std::vector<VkFence>(imageCount);
    imagesInFlight = std::vector<VkFence>(imageCount);

    VkSemaphoreCreateInfo semInfo{};
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (int i = 0; i < (int)imageCount; i++) {
        vkCreateSemaphore(device, &semInfo, nullptr, &imageAvailable[i]);
        vkCreateSemaphore(device, &semInfo, nullptr, &renderFinished[i]);
        vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]);
    }
    for (size_t i = 0; i < (size_t)imageCount; i++) {
        imagesInFlight[i] = VK_NULL_HANDLE;
    }
    

}

void App::createDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = imageCount * 10;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = imageCount * 10;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to creater descriptor pool!");
    }
}

App::App(const char* appName, const glm::vec2& windowSize) : objects(std::vector<Object>()){
    initGLFW(appName, windowSize.x, windowSize.y);
    initInstance(appName);
    initSurface();
    selectDevice();
    createSwapchain();
    createImageViews();
    createRenderPass();
    createFramebuffers();
    createCommandPool();
    createCommandBuffers();
    recordCommands();
    createSyncObjects();
    createDescriptorPool();
}
App::~App() {
    vkDeviceWaitIdle(device);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    for (Object object : objects) {
        object.destroy();
    }
    for (int i = 0; i < (int)imageCount; i++) {
        vkDestroySemaphore(device, imageAvailable[i], nullptr);
        vkDestroySemaphore(device, renderFinished[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(device, commandPool, nullptr);
    for (Object object : objects) {
        object.getPipeline()->destroy();
    }
    for (VkFramebuffer fb : framebuffers) {
        vkDestroyFramebuffer(device, fb, nullptr);
    }
    vkDestroyRenderPass(device, renderPass, nullptr);
    for (VkImageView view : imageViews) {
        vkDestroyImageView(device, view, nullptr);
    }
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);

    glfwTerminate();

}
void App::run() {
    
    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();

        // 1. Wait for the fence of this frame
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        // 2. Acquire swapchain image
        uint32_t imageIndex;
        vkAcquireNextImageKHR(
            device,
            swapchain,
            UINT64_MAX,
            imageAvailable[currentFrame],
            VK_NULL_HANDLE,
            &imageIndex
        );

        // 3. If the image is already in flight, wait for it
        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }

        // 4. Reset the fence **before** using it
        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        // 5. Mark image as now in use by this frame
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];

        // uniform test
        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), swapchainExtent.width / (float) swapchainExtent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;
        objects[0].setUBO(&ubo, imageIndex);

        // 6. Submit command buffer
        VkSubmitInfo submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailable[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = waitSemaphores;
        submit.pWaitDstStageMask = waitStages;

        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &commandBuffers[imageIndex];

        VkSemaphore signalSemaphores[] = { renderFinished[currentFrame] };
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphicsQueue, 1, &submit, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        // 7. Present
        VkPresentInfoKHR present{};
        present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present.waitSemaphoreCount = 1;
        present.pWaitSemaphores = signalSemaphores;
        present.swapchainCount = 1;
        present.pSwapchains = &swapchain;
        present.pImageIndices = &imageIndex;

        VkResult result = vkQueuePresentKHR(presentQueue, &present);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            // TODO: recreate swapchain here
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swapchain image!");
        }

        // 8. Advance frame index
        currentFrame = (currentFrame + 1) % imageCount;


        // print framerate
        static auto lastTime = std::chrono::high_resolution_clock::now();
        static int frameCount = 0;
            
        frameCount++;
            
        auto now = std::chrono::high_resolution_clock::now();
        float elapsed =
            std::chrono::duration<float>(now - lastTime).count();
            
        if (elapsed >= 1.0f) {
            float fps = frameCount / elapsed;
            std::cout << "FPS: " << fps << std::endl;
        
            frameCount = 0;
            lastTime = now;
        }
        
    }

}

RenderContext App::getRenderContext() const {
    return RenderContext{
        device,
        physicalDevice,
        renderPass,
        graphicsQueue,
        commandPool, 
        imageCount,
        descriptorPool
    };
}

VkExtent2D App::getSwapchainExtent() const {
    return swapchainExtent;
}

VkQueue App::getGraphicsQueue() const {
    return graphicsQueue;
}

VkCommandPool App::getCommandPool() const {
    return commandPool;
}

void App::addObject(const Object& object) {
    objects.push_back(object);
    recordCommands();
}