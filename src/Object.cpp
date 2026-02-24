#include "Object.hpp"
#include "Pipeline.hpp"
#include "RenderContext.hpp"
#include "Vertex.hpp"
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

Object::Object(const RenderContext& context, Pipeline* pipeline, const std::vector<Vertex>& vertices) : pipeline(pipeline), context(context), vertices(vertices){
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
    // Copy vertex data
    void* data;
    vkMapMemory(context.device, stagingMemory, 0, size, 0, &data);
    memcpy(data, vertices.data(), (size_t)size);
    vkUnmapMemory(context.device, stagingMemory);

    // 2️⃣ Create device-local vertex buffer
    createBuffer(
        size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vertexBuffer,
        vertexMemory
    );

    // 3️⃣ Copy staging → GPU buffer
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

void Object::draw(VkCommandBuffer cmd) const {
    
    VkBuffer buffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
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

    // 2️⃣ Get memory requirements
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context.device, buffer, &memRequirements);

    // find memory type
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

    // 3️⃣ Allocate memory
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    if (vkAllocateMemory(context.device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    // 4️⃣ Bind memory to buffer
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
    // 1️⃣ Allocate temporary command buffer
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    // 2️⃣ Begin recording (one-time submit)
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    // 3️⃣ Copy command
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

    // 4️⃣ Submit
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    // 5️⃣ Free command buffer
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}
Pipeline* Object::getPipeline() const {
    return pipeline;
}

void Object::destroy(){
    vkDestroyBuffer(context.device, vertexBuffer,  nullptr);
    vkFreeMemory(context.device, vertexMemory, nullptr);
}