#pragma once
#include "RenderContext.hpp"
#include "helpers.hpp"
#include <cstdint>
#include <vulkan/vulkan_core.h>


class IBLProcessor {
private:
    struct computePC {
        float roughness;
        uint32_t faceSize;
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
    VkDescriptorSet irradianceDS;


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
    VkDescriptorSet allocateComputeDS(const GPUImage& input, const GPUImage& output);


public:
    IBLProcessor(const RenderContext& ctx, const GPUImage& hdrImage);

    GPUImage createEnvironmentCubemap();
    GPUImage createIrradianceMap();
    GPUImage createPrefilterMap();
    GPUImage createBRDFLUT();

    void destroy();

};