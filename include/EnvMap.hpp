#pragma once
#include "RenderContext.hpp"
#include <string>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "helpers.hpp"
#include <glm/glm.hpp>


class EnvMap {
private:
    struct ImageInfo {
        float* data = nullptr;
        int width = 0;
        int height = 0;
        int channels = 0;
    };
    

    RenderContext context;
    std::string filePath;
    static constexpr uint32_t cubeFaceSize = 512;

    // Images
    GPUImage hdrImage;
    GPUImage environment;
    GPUImage irradiance;
    GPUImage prefilter;
    GPUImage brdfLUT;

    // Skybox rendering
    VkPipeline skyboxPipeline;
    VkPipelineLayout skyboxPipelineLayout;
    VkDescriptorSetLayout skyboxSetLayout;
    VkDescriptorPool skyboxDescriptorPool;
    VkDescriptorSet skyboxDescriptorSet;

    // Samplers
    VkSampler sampler;
    VkSampler brdfSampler;

    // Descriptor
    VkDescriptorSet descriptorSet;

    // Metadata
    uint32_t prefilterMipLevels;
    void init();
    ImageInfo loadImage();
    void createHDRImage();
    void createEnvironmentCubemap();
    void createIrradianceMap();
    void createPrefilterMap();
    void createBRDFLUT();
    void createSamplers();
    void createDescriptorSet();

    void createSkyboxDescriptorSetLayout();
    void createSkyboxPipeline();
    void createSkyboxDescriptor();

public:
    EnvMap(const RenderContext& context, const std::string& path);
    void renderSkybox(VkCommandBuffer cmd, VkExtent2D extent, glm::mat4 view, glm::mat4 proj);
    
    void destroy();
};