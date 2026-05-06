#pragma once
#include "RenderContext.hpp"
#include "UniformBufferObjects.hpp"
#include <vulkan/vulkan_core.h>
#include <string>
#include <vector>

class Pipeline {

private: 
    RenderContext context;
    std::string vert, frag;

    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;

    void create(BRDF brdf = 0);

    void createLayout();

public:

    Pipeline(const RenderContext& context, const std::string& vert, const std::string&frag);

    const std::vector<VkDescriptorSetLayout>& getDescriptorSetLayouts() const { return descriptorSetLayouts;}

    VkPipelineLayout getPipelineLayout() const { return pipelineLayout;}

    void bind(VkCommandBuffer cmd, const VkExtent2D& extent) const;

    void recreate(BRDF brdf = 0);

    void destroy();

};