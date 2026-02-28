#pragma once

#include "Camera.hpp"
#include "Pipeline.hpp"
#include "RenderContext.hpp"
#include "Vertex.hpp"
#include "VertexUBO.hpp"
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

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    std::vector<VkDescriptorSet> descriptorSets;

    VertexUBO vertexUBO;

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

    void update(const Camera& camera, size_t index) {
        vertexUBO.model = glm::identity<glm::mat4>();
        vertexUBO.view = camera.view();
        vertexUBO.proj = camera.proj();

        memcpy(uniformBuffersMapped[index], &vertexUBO, sizeof(VertexUBO));
    }

    void destroy();

};