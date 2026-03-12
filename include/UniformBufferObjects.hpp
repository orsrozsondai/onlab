#pragma once

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/mat4x4.hpp>

struct MaterialUBO {
    alignas(16) glm::vec3 albedo;
    float metallic;
    float roughness;



    MaterialUBO() {
        albedo = {0,0,0};
        metallic = 0;
        roughness = 0;
    }

};

struct SceneUBO {
    alignas(16) glm::vec4 lightPos;
    alignas(16) glm::vec3 lightColor;

    alignas(16) glm::vec3 camPos;
};

struct MVP_UBO {
    glm::mat4 model, view, proj;
};