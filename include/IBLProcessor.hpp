#pragma once
#include "RenderContext.hpp"
#include "helpers.hpp"
#include <cstdint>
#include <vulkan/vulkan_core.h>


class IBLProcessor {
private:
    struct computePC {
        float roughness;
        int mipLevel;
        int faceSize;
        // int pad;
    };

    GPUImage hdrImage;
    VkSampler hdrSampler;
    RenderContext context;
    VkDescriptorSetLayout hdrDescriptorLayout;
    VkDescriptorPool hdrDescriptorPool;
    VkDescriptorSet hdrDescriptorSet;
    VkRenderPass renderPass;
    VkPipelineLayout hdrPipelineLayout;

    GPUImage envMap;

    VkDescriptorSetLayout computeDSL;
    VkPipelineLayout computePL;
    VkSampler cubeSampler;


    VkPipeline eqToCubePipeline;
    VkPipeline irradiancePipeline;
    VkPipeline prefilterPipeline;
    VkPipeline brdfPipeline;
    static constexpr uint32_t faceSize = 2048;

    void createRenderPass();
    void createHDRSampler();
    void createEqToCubePipeline();
    void createHDRDescriptorPool();
    void createHDRDescriptorSetLayout();
    void allocateHDRDescriptorSet();
    void updateHDRDescriptorSet();

    void createComputeDSL();
    void createComputePL();
    VkPipeline createComputePipeline(const std::string& path);
    void createCubeSampler();
    VkDescriptorSet allocateComputeDS(VkImageView input, VkImageView output);
    void updateComputeDS(VkDescriptorSet DS, VkImageView input, VkImageView output);


public:
    IBLProcessor(const RenderContext& ctx, const GPUImage& hdrImage);

    GPUImage createEnvironmentCubemap();
    GPUImage createIrradianceMap();
    GPUImage createPrefilterMap();
    GPUImage createBRDFLUT();

    void destroy();

};