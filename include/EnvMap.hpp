#pragma once
#include "RenderContext.hpp"
#include <string>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "helpers.hpp"


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

public:
    EnvMap(const RenderContext& context, const std::string& path);
    
    void destroy();
};