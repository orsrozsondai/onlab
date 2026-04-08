#pragma once
#include <glm/ext/vector_float3.hpp>
#include <vector>
#include <memory>
#include <vulkan/vulkan_core.h>
#include "Camera.hpp"
#include "EnvMap.hpp"
#include "MeshLoader.hpp"
#include "Object.hpp"
#include "Pipeline.hpp"
#include "RenderContext.hpp"
#include "UniformBufferObjects.hpp"

class Scene {
private:
    int currentObjectCount = 3;
    float objectDistance = 2;
    int meshIndex = 0;
    int selectedObjectIndex = 0;
    RenderContext context;
    std::vector<Object*> objects;
    SceneUBO sceneUBO;
    std::vector<std::unique_ptr<MeshLoader>> meshes;
    std::vector<VkDescriptorSet> descriptorSets;
    Pipeline* pipeline;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    Camera* camera;
    EnvMap* env = nullptr;

    void createDescriptorSets();
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void createUniformBuffers();
    void createObjects();
    void destroyObjects();
    void setMeshIndex(int index);
    void arrangeObjects();
    void interpolateFloat(MaterialParameters param);
    void interpolateVec3(MaterialParameters param);

public:
    Scene(const RenderContext& context, Pipeline* pPipeline, Camera* pCamera);

    void update();
    void draw(VkCommandBuffer cmd, VkExtent2D extent, size_t frameIndex);
    void addMesh(std::unique_ptr<MeshLoader> mesh);
    Pipeline* getPipeline() const;
    SceneUBO* ubo();
    Object* selectedObject();
    void selectObject(int index);
    void cycleSelected(int dir); // -1 = left, 1 = right
    void setObjectCount(int c);
    void setObjectDistance(float d);
    void interpolate(MaterialParameters param);
    bool isObjectInterpolated();
    void addEnvMap(EnvMap* pEnv);
    void destroy();


    Object* addObject(std::unique_ptr<Object> obj);
};