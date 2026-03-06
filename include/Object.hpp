#pragma once

#include "Camera.hpp"
#include "Pipeline.hpp"
#include "RenderContext.hpp"
#include "Vertex.hpp"
#include "VertexUBO.hpp"
#include "FragmentUBO.hpp"
#include <cstddef>
#include <cstring>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

class Object {
private:
    Pipeline* pipeline;
    RenderContext context;
    std::vector<Vertex> vertices;

    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexMemory = VK_NULL_HANDLE;

    std::vector<VkBuffer> vs_uniformBuffers;
    std::vector<VkDeviceMemory> vs_uniformBuffersMemory;
    std::vector<void*> vs_uniformBuffersMapped;
    std::vector<VkBuffer> fs_uniformBuffers;
    std::vector<VkDeviceMemory> fs_uniformBuffersMemory;
    std::vector<void*> fs_uniformBuffersMapped;
    std::vector<VkDescriptorSet> descriptorSets;

    VertexUBO vertexUBO;
    FragmentUBO fragmentUBO;

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    void copyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void createVertexBuffer();

    void uploadVertices();

    void createUniformBuffers();

    void createDescriptorSets();

    public:

    Object(const RenderContext& context, Pipeline* pipeline, const std::vector<Vertex>& vertices);

    void draw(VkCommandBuffer cmd, size_t frameIndex) const;

    Pipeline* getPipeline() const;

    void update(const Camera& camera);

    FragmentUBO* ubo();


    void destroy();

};