#pragma once
#include "RenderContext.hpp"
#include "helpers.hpp"
#include <cstdint>
#include <vulkan/vulkan_core.h>


class IBLProcessor {
private:
    GPUImage hdrImage;
    VkSampler hdrSampler;
    RenderContext context;
    VkDescriptorSetLayout hdrDescriptorLayout;
    VkDescriptorPool hdrDescriptorPool;
    VkDescriptorSet hdrDescriptorSet;
    VkRenderPass renderPass;
    VkPipelineLayout hdrPipelineLayout;

    VkPipeline eqToCubePipeline;
    VkPipeline irradiancePipeline;
    VkPipeline prefilterPipeline;
    VkPipeline brdfPipeline;
    static constexpr uint32_t faceSize = 1024;

    void createHDRSampler();
    void createEqToCubePipeline();
    void createHDRDescriptorPool();
    void createHDRDescriptorSetLayout();
    void allocateHDRDescriptorSet();
    void updateHDRDescriptorSet();

public:
    IBLProcessor(const RenderContext& ctx, const GPUImage& hdrImage);
    void createRenderPass();
    GPUImage createEnvironmentCubemap();
    GPUImage createIrradianceMap(GPUImage env);
    GPUImage createPrefilterMap(GPUImage env, uint32_t& mipLevels);
    GPUImage createBRDFLUT();

    void destroy();

};