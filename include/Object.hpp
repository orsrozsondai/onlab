#pragma once

#include "Pipeline.hpp"
#include "RenderContext.hpp"
#include "Vertex.hpp"
#include <glm/ext/vector_float3.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>

class Object {
private:
    Pipeline* pipeline;
    RenderContext context;
    std::vector<Vertex> vertices;

    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexMemory = VK_NULL_HANDLE;

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    void copyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
public:
    Object(const RenderContext& context, Pipeline* pipeline, const std::vector<Vertex>& vertices);

    void draw(VkCommandBuffer cmd) const;

    Pipeline* getPipeline() const;

    void destroy();

};