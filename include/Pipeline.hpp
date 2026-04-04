#pragma once
#include "RenderContext.hpp"
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

    void create();
    static std::vector<char> readFile(const std::string& path);
    VkShaderModule createShaderModule(const std::vector<char>& code); 

public:

    Pipeline(const RenderContext& context, const std::string& vert, const std::string&frag);

    const std::vector<VkDescriptorSetLayout>& getDescriptorSetLayouts() const { return descriptorSetLayouts;}

    VkPipelineLayout getPipelineLayout() const { return pipelineLayout;}

    void bind(VkCommandBuffer cmd, const VkExtent2D& extent) const;

    void destroy();

};