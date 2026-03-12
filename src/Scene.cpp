#include "Scene.hpp"
#include "Camera.hpp"
#include "MeshLoader.hpp"
#include "Object.hpp"
#include "Pipeline.hpp"
#include "RenderContext.hpp"
#include "UniformBufferObjects.hpp"
#include <memory>
#include <vector>
#include "Config.hpp"

Scene::Scene(const RenderContext& context, Pipeline* pPipeline, Camera* pCamera) : context(context), pipeline(pPipeline), camera(pCamera) {
    meshes = std::vector<std::unique_ptr<MeshLoader>>();
    objects = std::vector<std::unique_ptr<Object>>();
    createUniformBuffers();
    createDescriptorSets();
    // createObjects();
}

Object* Scene::addObject(std::unique_ptr<Object> obj) {
    objects.push_back(std::move(obj));
    return objects.back().get();
}

void Scene::addMesh(std::unique_ptr<MeshLoader> mesh) {
    meshes.push_back(std::move(mesh));
    if (objects.empty()) createObjects();
}

void Scene::createDescriptorSets() {
    descriptorSets = std::vector<VkDescriptorSet>(context.imageCount);
    std::vector<VkDescriptorSetLayout> layouts(
        context.imageCount,
        pipeline->getDescriptorSetLayout()
    );
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = context.descriptorPool;
    allocInfo.descriptorSetCount = layouts.size();
    allocInfo.pSetLayouts = layouts.data();

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
        bufferInfo.range  = sizeof(SceneUBO);


        std::array<VkWriteDescriptorSet, 1> writes{};
        writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[0].dstSet = descriptorSets[i];
        writes[0].dstBinding = 2; 
        writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writes[0].descriptorCount = 1;
        writes[0].pBufferInfo = &bufferInfo;


        vkUpdateDescriptorSets(
            context.device,
            writes.size(),
            writes.data(),
            0, nullptr
        );
    }
}

void Scene::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
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

void Scene::createUniformBuffers() {
    uniformBuffers = std::vector<VkBuffer>(context.imageCount);
    uniformBuffersMemory = std::vector<VkDeviceMemory>(context.imageCount);
    uniformBuffersMapped = std::vector<void*>(context.imageCount);

    VkDeviceSize size = sizeof(SceneUBO);

    for (size_t i = 0; i < context.imageCount; i++) {
        createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
        vkMapMemory(context.device, uniformBuffersMemory[i], 0, size, 0, &uniformBuffersMapped[i]);
    }
}

void Scene::createObjects() {
    for (int i = 0; i < MAX_OBJECT_COUNT; i++) {
        objects.push_back(std::make_unique<Object>(Object(context, pipeline, meshes[meshIndex].get())));
    }
}

void Scene::destroyObjects() {
    for (auto& object : objects) {
        if(object.get()){
            object.get()->destroy();
            // object.reset();
        }
    }
    //objects.clear();
}

void Scene::setMeshIndex(int index) {
    meshIndex = index;
    destroyObjects();
    createObjects();
}

void Scene::update() {
    for (int i = 0; i < currentObjectCount; i++) {
        objects[i].get()->update(*camera);
    }
}
void Scene::draw(VkCommandBuffer cmd, size_t frameIndex) {
    for (int i = 0; i < currentObjectCount; i++) {
        objects[i].get()->draw(cmd, frameIndex);
    }
}

Pipeline* Scene::getPipeline() const {
    return pipeline;
}

SceneUBO* Scene::ubo() {
    return &sceneUBO;
}

Object* Scene::selectedObject() {
    return objects[selectedObjectIndex].get();
}

void Scene::selectObject(int index) {
    selectedObjectIndex = index;
    camera->lookAt(selectedObject()->getPosition());
}

void Scene::destroy() {
    destroyObjects();
    pipeline->destroy();
}