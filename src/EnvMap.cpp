#include "EnvMap.hpp"
#include "IBLProcessor.hpp"
#include "RenderContext.hpp"
#include <array>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vulkan/vulkan_core.h>
#include "helpers.hpp"
#include "stb_image.h"
#include <iostream>

EnvMap::EnvMap(const RenderContext& context, const std::string& path) : context(context), filePath(path) {
    init();
}

EnvMap::ImageInfo EnvMap::loadImage() {
    ImageInfo res;

    res.data = stbi_loadf(filePath.c_str(), &res.width, &res.height, &res.channels, STBI_rgb_alpha);
    if (res.data)
        std::cout << "Image loaded: " << res.width << " x " << res.height << std::endl;
    else
        throw std::runtime_error("Failed to load image: " + filePath);
    return res;
}

void EnvMap::init() {
    createHDRImage();
    IBLProcessor proc(context, hdrImage);
}

void EnvMap::createHDRImage() {

    ImageInfo image = loadImage();
    VkDeviceSize imageSize = image.width * image.height * 4 * sizeof(float);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;


    createBuffer(
        context.device,
        context.physicalDevice,
        imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingMemory
    );


    void *data;
    vkMapMemory(context.device, stagingMemory, 0, imageSize, 0, &data);
    memcpy(data, image.data, imageSize);
    vkUnmapMemory(context.device, stagingMemory);

    createImage(
        context.device,
        context.physicalDevice,
        image.width,
        image.height,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        0,
        hdrImage.image,
        hdrImage.memory
    );

    transitionImageLayout(
        context.device,
        context.commandPool,
        context.graphicsQueue,
        hdrImage.image,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );

    
    copyBufferToImage(
        context.device, 
        context.commandPool, 
        context.graphicsQueue, 
        stagingBuffer, 
        hdrImage.image, 
        image.width, 
        image.height
    );

    transitionImageLayout(
        context.device,
        context.commandPool,
        context.graphicsQueue,
        hdrImage.image,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );


    hdrImage.view = createImageView(
        context.device,
        hdrImage.image,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_IMAGE_ASPECT_COLOR_BIT,
        1,
        1,
        VK_IMAGE_VIEW_TYPE_2D
    );

    vkDestroyBuffer(context.device, stagingBuffer, nullptr);
    vkFreeMemory(context.device, stagingMemory, nullptr);

    stbi_image_free(image.data);
}

void EnvMap::createEnvironmentCubemap() {

    /* createImage(
        context.device,
        context.physicalDevice,
        cubeFaceSize,
        cubeFaceSize,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
        environment.image,
        environment.memory,
        1,
        6
    );

    environment.view = createImageView(
        context.device,
        environment.image,
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
        environment.image,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        1,
        6
    ); */

}

void EnvMap::createDescriptorSet() {
    std::array<VkDescriptorSetLayoutBinding, 4> bindings{};

    // irradiance map
    bindings[0] = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };

    // prefilter map
    bindings[1] = {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };

    // BRDF LUT
    bindings[2] = {
        .binding = 2,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };

    // Environment
    bindings[2] = {
        .binding = 3,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };

    VkDescriptorImageInfo irradianceInfo {
        sampler,
        irradiance.view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkDescriptorImageInfo prefilterInfo {
        sampler,
        prefilter.view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkDescriptorImageInfo brdfInfo {
        brdfSampler,
        brdfLUT.view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkDescriptorImageInfo envInfo {
        sampler,
        environment.view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    std::array<VkWriteDescriptorSet, 4> writes{};

    writes[0] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &irradianceInfo
    };

    writes[1] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = 1,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &prefilterInfo
    };

    writes[2] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = 2,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &brdfInfo
    };
    writes[3] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = 3,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &envInfo
    };

    vkUpdateDescriptorSets(context.device, writes.size(), writes.data(), 0, nullptr);
}