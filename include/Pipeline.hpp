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

    void create();
    static std::vector<char> readFile(const std::string& path);
    VkShaderModule createShaderModule(const std::vector<char>& code); 

public:

    Pipeline(const RenderContext& context, const std::string& vert, const std::string&frag);


    void bind(VkCommandBuffer cmd) const;

    void destroy();

};