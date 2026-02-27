#include "Object.hpp"
#include "Pipeline.hpp"
#include "RenderContext.hpp"
#include "Vertex.hpp"
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>
#include "UniformBufferObject.hpp"
#include <vulkan/vulkan_core.h>

Object::Object(const RenderContext& context, Pipeline* pipeline, const std::vector<Vertex>& vertices) : pipeline(pipeline), context(context), vertices(vertices){
    createVertexBuffer();
    uploadVertices();
    createUniformBuffers();
    createDescriptorSets();
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
)   ;
    
    
    vkCmdDraw(cmd, vertices.size(), 1, 0, 0);
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
    for (size_t i = 0; i < context.imageCount; i++) {
        vkDestroyBuffer(context.device, uniformBuffers[i], nullptr);
        vkFreeMemory(context.device, uniformBuffersMemory[i], nullptr);
    }


}

void Object::createUniformBuffers() {
    uniformBuffers = std::vector<VkBuffer>(context.imageCount);
    uniformBuffersMemory = std::vector<VkDeviceMemory>(context.imageCount);
    uniformBuffersMapped = std::vector<void*>(context.imageCount);

    VkDeviceSize size = sizeof(UniformBufferObject);

    for (size_t i = 0; i < context.imageCount; i++) {
        createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
        vkMapMemory(context.device, uniformBuffersMemory[i], 0, size, 0, &uniformBuffersMapped[i]);

    }

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
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range  = sizeof(UniformBufferObject);

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = descriptorSets[i];
        write.dstBinding = 0; 
        write.dstArrayElement = 0;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.descriptorCount = 1;
        write.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(
            context.device,
            1,
            &write,
            0,
            nullptr
        );
    }
}