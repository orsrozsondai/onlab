#include "Object.hpp"
#include "FragmentUBO.hpp"
#include "Pipeline.hpp"
#include "RenderContext.hpp"
#include "Vertex.hpp"
#include <cstdint>
#include <cstring>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float4.hpp>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <vector>
#include <array>
#include "VertexUBO.hpp"
#include <vulkan/vulkan_core.h>

Object::Object(const RenderContext& context, Pipeline* pipeline, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) 
        : pipeline(pipeline), context(context), vertices(vertices), indices(indices) {
    createVertexBuffer();
    uploadVertices();
    createIndexBuffer();
    uploadIndices();
    createUniformBuffers();
    createDescriptorSets();
    updateModelMat();
}

void Object::createVertexBuffer() {
    VkDeviceSize size = sizeof(vertices[0]) * vertices.size();
    createBuffer(
        size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vertexBuffer,
        vertexMemory
    );
}

void Object::createIndexBuffer() {
    VkDeviceSize size = sizeof(indices[0]) * indices.size();
    createBuffer(
        size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        indexBuffer,
        indexMemory
    );
}

void Object::uploadVertices() {
    VkDeviceSize size = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    createBuffer(
        size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingMemory
    );
    
    void* data;
    vkMapMemory(context.device, stagingMemory, 0, size, 0, &data);
    memcpy(data, vertices.data(), (size_t)size);
    vkUnmapMemory(context.device, stagingMemory);

    copyBuffer(
        context.device,
        context.commandPool,
        context.graphicsQueue,
        stagingBuffer,
        vertexBuffer,
        size
    );

    vkDestroyBuffer(context.device, stagingBuffer, nullptr);
    vkFreeMemory(context.device, stagingMemory, nullptr);

}

void Object::uploadIndices() {
    VkDeviceSize size = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    createBuffer(
        size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingMemory
    );
    
    void* data;
    vkMapMemory(context.device, stagingMemory, 0, size, 0, &data);
    memcpy(data, indices.data(), (size_t)size);
    vkUnmapMemory(context.device, stagingMemory);

    copyBuffer(
        context.device,
        context.commandPool,
        context.graphicsQueue,
        stagingBuffer,
        indexBuffer,
        size
    );

    vkDestroyBuffer(context.device, stagingBuffer, nullptr);
    vkFreeMemory(context.device, stagingMemory, nullptr);

}

void Object::draw(VkCommandBuffer cmd, size_t frameIndex) const {
    VkBuffer buffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
    vkCmdBindDescriptorSets(
       cmd,
       VK_PIPELINE_BIND_POINT_GRAPHICS,
       pipeline->getPipelineLayout(),
       0,                     
       1,
       &descriptorSets[frameIndex],
       0,
       nullptr
    );
    
    vkCmdBindIndexBuffer(cmd, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    
    // vkCmdDraw(cmd, vertices.size(), 1, 0, 0);
    vkCmdDrawIndexed(cmd, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}

void Object::update(const Camera& camera) {
    vertexUBO.view = camera.view();
    vertexUBO.proj = camera.proj();
    for (int index = 0; index < context.imageCount; index++) {
        memcpy(vs_uniformBuffersMapped[index], &vertexUBO, sizeof(VertexUBO));
        memcpy(fs_uniformBuffersMapped[index], &fragmentUBO, sizeof(FragmentUBO));
    }
}



void Object::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(context.device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context.device, buffer, &memRequirements);

    uint32_t memoryTypeIndex;
    bool found = false;
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(
        context.physicalDevice,
        &memProperties
    );

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((memRequirements.memoryTypeBits & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            memoryTypeIndex = i;
            found = true;
            break;
        }
    }

    if (!found) throw std::runtime_error("failed to find suitable memory type!");

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    if (vkAllocateMemory(context.device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(context.device, buffer, bufferMemory, 0);
}

void Object::copyBuffer(
    VkDevice device,
    VkCommandPool commandPool,
    VkQueue graphicsQueue,
    VkBuffer srcBuffer,
    VkBuffer dstBuffer,
    VkDeviceSize size
) {
    
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;

    vkCmdCopyBuffer(
        commandBuffer,
        srcBuffer,
        dstBuffer,
        1,
        &copyRegion
    );

    vkEndCommandBuffer(commandBuffer);

    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}
Pipeline* Object::getPipeline() const {
    return pipeline;
}

void Object::destroy(){
    vkDestroyBuffer(context.device, vertexBuffer,  nullptr);
    vkFreeMemory(context.device, vertexMemory, nullptr);
    vkDestroyBuffer(context.device, indexBuffer, nullptr);
    vkFreeMemory(context.device, indexMemory, nullptr);
    for (size_t i = 0; i < context.imageCount; i++) {
        if (vs_uniformBuffers[i] != VK_NULL_HANDLE) vkDestroyBuffer(context.device, vs_uniformBuffers[i], nullptr);
        vs_uniformBuffers[i] = VK_NULL_HANDLE;
        if (vs_uniformBuffersMemory[i] != VK_NULL_HANDLE) vkFreeMemory(context.device, vs_uniformBuffersMemory[i], nullptr);
        vs_uniformBuffersMemory[i] = VK_NULL_HANDLE;
        if (fs_uniformBuffers[i] != VK_NULL_HANDLE) vkDestroyBuffer(context.device, fs_uniformBuffers[i], nullptr);
        fs_uniformBuffers[i] = VK_NULL_HANDLE;
        if (fs_uniformBuffersMemory[i] != VK_NULL_HANDLE) vkFreeMemory(context.device, fs_uniformBuffersMemory[i], nullptr);
        fs_uniformBuffersMemory[i] = VK_NULL_HANDLE;
    }
}

void Object::createUniformBuffers() {
    vs_uniformBuffers = std::vector<VkBuffer>(context.imageCount);
    vs_uniformBuffersMemory = std::vector<VkDeviceMemory>(context.imageCount);
    vs_uniformBuffersMapped = std::vector<void*>(context.imageCount);

    VkDeviceSize size = sizeof(VertexUBO);

    for (size_t i = 0; i < context.imageCount; i++) {
        createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vs_uniformBuffers[i], vs_uniformBuffersMemory[i]);
        vkMapMemory(context.device, vs_uniformBuffersMemory[i], 0, size, 0, &vs_uniformBuffersMapped[i]);

    }
    fs_uniformBuffers = std::vector<VkBuffer>(context.imageCount);
    fs_uniformBuffersMemory = std::vector<VkDeviceMemory>(context.imageCount);
    fs_uniformBuffersMapped = std::vector<void*>(context.imageCount);

    size = sizeof(FragmentUBO);

    for (size_t i = 0; i < context.imageCount; i++) {
        createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, fs_uniformBuffers[i], fs_uniformBuffersMemory[i]);
        vkMapMemory(context.device, fs_uniformBuffersMemory[i], 0, size, 0, &fs_uniformBuffersMapped[i]);

    }
}

FragmentUBO* Object::ubo() {
    return &fragmentUBO;
}

void Object::createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(
        context.imageCount,
        pipeline->getDescriptorSetLayout()
    );

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = context.descriptorPool;
    allocInfo.descriptorSetCount = layouts.size();
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets = std::vector<VkDescriptorSet>(context.imageCount);

    vkAllocateDescriptorSets(
        context.device,
        &allocInfo,
        descriptorSets.data()
    );

    for (size_t i = 0; i < context.imageCount; i++)
    {
        VkDescriptorBufferInfo vsBufferInfo{};
        vsBufferInfo.buffer = vs_uniformBuffers[i];
        vsBufferInfo.offset = 0;
        vsBufferInfo.range  = sizeof(VertexUBO);

        VkDescriptorBufferInfo fsBufferInfo{};
        fsBufferInfo.buffer = fs_uniformBuffers[i];
        fsBufferInfo.offset = 0;
        fsBufferInfo.range  = sizeof(FragmentUBO);

        std::array<VkWriteDescriptorSet, 2> writes{};
        // VkWriteDescriptorSet write{};
        writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[0].dstSet = descriptorSets[i];
        writes[0].dstBinding = 0; 
        // writes[0].dstArrayElement = 0;
        writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writes[0].descriptorCount = 1;
        writes[0].pBufferInfo = &vsBufferInfo;

        writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[1].dstSet = descriptorSets[i];
        writes[1].dstBinding = 1;
        writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writes[1].descriptorCount = 1;
        writes[1].pBufferInfo = &fsBufferInfo;

        vkUpdateDescriptorSets(
            context.device,
            writes.size(),
            writes.data(),
            0, nullptr
        );
    }
}

void Object::setScale(float scale) {
    this->scale = scale;
    updateModelMat();
}
void Object::setPosition(const glm::vec3& pos) {
    position = pos;
    updateModelMat();
}
void Object::updateModelMat() {
    vertexUBO.model = glm::mat4(1.0f);
    vertexUBO.model = glm::translate(vertexUBO.model, position);
    vertexUBO.model = glm::scale(vertexUBO.model, glm::vec3(scale));
}