#pragma once

#include "Pipeline.hpp"
#include "RenderContext.hpp"
#include "UniformBufferObject.hpp"
#include "Vertex.hpp"
#include <cstddef>
#include <cstring>
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

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    std::vector<VkDescriptorSet> descriptorSets;

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    void copyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void createUniformBuffers();

    void createDescriptorSets();

    public:

    Object(const RenderContext& context, Pipeline* pipeline, const std::vector<Vertex>& vertices);

    void draw(VkCommandBuffer cmd, size_t frameIndex) const;

    Pipeline* getPipeline() const;

    void setUBO(UniformBufferObject* ubo, size_t index) {
        memcpy(uniformBuffersMapped[index], ubo, sizeof(UniformBufferObject ));
    }

    void destroy();

};