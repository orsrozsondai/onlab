#include "IBLProcessor.hpp"
#include "RenderContext.hpp"
#include "helpers.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

IBLProcessor::IBLProcessor(const RenderContext& context, const GPUImage& hdrImage) : context(context), hdrImage(hdrImage) {
    createRenderPass();
    createHDRSampler();
    createHDRDescriptorPool();
    createHDRDescriptorSetLayout();
    allocateHDRDescriptorSet();
    createEqToCubePipeline();
    updateHDRDescriptorSet();
}

void IBLProcessor::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = VK_FORMAT_R16G16B16A16_SFLOAT; // HDR
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    colorAttachment.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments    = &colorAttachment;
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies   = &dependency;

    if (vkCreateRenderPass(context.device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create cubemap render pass!");
    }
}

void IBLProcessor::createHDRSampler() {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    
    samplerInfo.magFilter    = VK_FILTER_LINEAR;
    samplerInfo.minFilter    = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode   = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    samplerInfo.mipLodBias   = 0.0f;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy    = 1.0f;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp     = VK_COMPARE_OP_ALWAYS;

    samplerInfo.minLod       = 0.0f;
    samplerInfo.maxLod       = VK_LOD_CLAMP_NONE;
    samplerInfo.borderColor  = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    if (vkCreateSampler(context.device, &samplerInfo, nullptr, &hdrSampler) != VK_SUCCESS)
        throw std::runtime_error("Failed to create HDR sampler!");

}

void IBLProcessor::createHDRDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes    = &poolSize;
    poolInfo.maxSets       = 1;

    
    if (vkCreateDescriptorPool(context.device, &poolInfo, nullptr, &hdrDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create HDR descriptor pool!");
    }

    ;
}

void IBLProcessor::createHDRDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding binding{};
    binding.binding            = 0;
    binding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount    = 1;
    binding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
    binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings    = &binding;

    if (vkCreateDescriptorSetLayout(context.device, &layoutInfo, nullptr, &hdrDescriptorLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create HDR descriptor set layout!");
    }

}

void IBLProcessor::allocateHDRDescriptorSet() {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = hdrDescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts        = &hdrDescriptorLayout;

    if (vkAllocateDescriptorSets(context.device, &allocInfo, &hdrDescriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate HDR descriptor set!");
    }

}

void IBLProcessor::updateHDRDescriptorSet() {
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView   = hdrImage.view;
    imageInfo.sampler     = hdrSampler;

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet          = hdrDescriptorSet;
    descriptorWrite.dstBinding      = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo      = &imageInfo;

    vkUpdateDescriptorSets(context.device, 1, &descriptorWrite, 0, nullptr);
}

void IBLProcessor::createEqToCubePipeline() {
    VkShaderModule vertModule = createShaderModule(context.device, readFile("build/shaders/eqToCube.vert.spv"));
    VkShaderModule fragModule = createShaderModule(context.device, readFile("build/shaders/eqToCube.frag.spv"));

    VkPipelineShaderStageCreateInfo vertStage{};
    vertStage.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStage.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vertStage.module = vertModule;
    vertStage.pName  = "main";

    VkPipelineShaderStageCreateInfo fragStage{};
    fragStage.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStage.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStage.module = fragModule;
    fragStage.pName  = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertStage, fragStage };

    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount   = 0;
    vertexInput.pVertexBindingDescriptions      = nullptr;
    vertexInput.vertexAttributeDescriptionCount = 0;
    vertexInput.pVertexAttributeDescriptions    = nullptr;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = faceSize;
    viewport.height   = faceSize;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkExtent2D extent;
    extent.height = faceSize;
    extent.width = faceSize;
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = extent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports    = &viewport;
    viewportState.scissorCount  = 1;
    viewportState.pScissors     = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = VK_CULL_MODE_NONE; // flip if cube appears inside-out
    rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                               VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable         = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable     = VK_FALSE;
    colorBlending.attachmentCount   = 1;
    colorBlending.pAttachments      = &colorBlendAttachment;


    VkPushConstantRange range{};
    range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    range.offset = 0;
    range.size = 2 * sizeof(glm::mat4);
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts    = &hdrDescriptorLayout; // for equirect sampler
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &range;

    if (vkCreatePipelineLayout(context.device, &pipelineLayoutInfo, nullptr, &hdrPipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create eqToCube pipeline layout");

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount          = 2;
    pipelineInfo.pStages             = shaderStages;
    pipelineInfo.pVertexInputState   = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState   = &multisampling;
    pipelineInfo.pDepthStencilState  = nullptr;
    pipelineInfo.pColorBlendState    = &colorBlending;
    pipelineInfo.layout              = hdrPipelineLayout;
    pipelineInfo.renderPass          = renderPass;
    pipelineInfo.subpass             = 0;

    if (vkCreateGraphicsPipelines(context.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &eqToCubePipeline) != VK_SUCCESS)
        throw std::runtime_error("Failed to create eqToCube pipeline");

    vkDestroyShaderModule(context.device, vertModule, nullptr);
    vkDestroyShaderModule(context.device, fragModule, nullptr);
}

GPUImage IBLProcessor::createEnvironmentCubemap() {
    GPUImage cubemap;

    createImage(
        context.device,
        context.physicalDevice,
        faceSize,
        faceSize,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
        cubemap.image,
        cubemap.memory,
        1,
        6 
    );

    cubemap.view = createImageView(
        context.device,
        cubemap.image,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_IMAGE_ASPECT_COLOR_BIT,
        1,
        6,
        VK_IMAGE_VIEW_TYPE_CUBE
    );

    transitionImageLayout(
        context.device,
        context.commandPool,
        context.graphicsQueue,
        cubemap.image,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        1,
        6
    );
    

    std::vector<VkFramebuffer> framebuffers(6);
    std::vector<VkImageView> faceViews(6);
    for (uint32_t face = 0; face < 6; ++face) {
        faceViews[face] = createCubemapFaceView(
            context.device,
            cubemap.image,
            VK_FORMAT_R16G16B16A16_SFLOAT,
            0,
            face
        );

        framebuffers[face] = createFramebuffer(
            context.device,
            renderPass,
            faceViews[face],
            faceSize,
            faceSize
        );
    }

    VkCommandBuffer cmd = beginSingleTimeCommands(context.device, context.commandPool);

    glm::mat4 captureProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[6] = {
        glm::lookAt(glm::vec3(0), glm::vec3(1,0,0),   glm::vec3(0,-1,0)),
        glm::lookAt(glm::vec3(0), glm::vec3(-1,0,0),  glm::vec3(0,-1,0)),
        glm::lookAt(glm::vec3(0), glm::vec3(0,1,0),   glm::vec3(0,0,1)),
        glm::lookAt(glm::vec3(0), glm::vec3(0,-1,0),  glm::vec3(0,0,-1)),
        glm::lookAt(glm::vec3(0), glm::vec3(0,0,1),   glm::vec3(0,-1,0)),
        glm::lookAt(glm::vec3(0), glm::vec3(0,0,-1),  glm::vec3(0,-1,0))
    };

    for (uint32_t face = 0; face < 6; ++face) {
        VkClearValue clearValue{};
        clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass  = renderPass;
        renderPassInfo.framebuffer = framebuffers[face];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = {faceSize, faceSize};
        renderPassInfo.clearValueCount   = 1;
        renderPassInfo.pClearValues      = &clearValue;

        vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, eqToCubePipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, hdrPipelineLayout,
                                0, 1, &hdrDescriptorSet, 0, nullptr);

        vkCmdPushConstants(cmd, hdrPipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT,
                           0, sizeof(glm::mat4), &captureViews[face]);
        vkCmdPushConstants(cmd, hdrPipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT,
                           sizeof(glm::mat4), sizeof(glm::mat4), &captureProj);

        vkCmdDraw(cmd, 36, 1, 0, 0);

        vkCmdEndRenderPass(cmd);
    }
    
    endSingleTimeCommands(context.device, context.commandPool, context.graphicsQueue, cmd);

    transitionImageLayout(
        context.device,
        context.commandPool,
        context.graphicsQueue,
        cubemap.image,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        1,
        6
    );

    
    for (auto fb : framebuffers)
        vkDestroyFramebuffer(context.device, fb, nullptr);

    for (auto fv : faceViews)
        vkDestroyImageView(context.device, fv, nullptr);

    return cubemap;
}

void IBLProcessor::destroy() {
    vkDeviceWaitIdle(context.device);
    vkDestroyPipeline(context.device, eqToCubePipeline, nullptr);
    vkDestroyPipelineLayout(context.device, hdrPipelineLayout, nullptr);
    vkDestroyDescriptorPool(context.device, hdrDescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(context.device, hdrDescriptorLayout, nullptr);
    vkDestroyRenderPass(context.device, renderPass, nullptr);
    vkDestroySampler(context.device, hdrSampler, nullptr);
}