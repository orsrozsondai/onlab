#pragma once

#include <glm/ext/matrix_float4x4.hpp>
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

    SceneUBO() {
        lightPos = {0, -1, 0, 0};
        lightColor = {1, 1, 1};
        camPos = {0, 0, 0};
    }
};

struct MVP_UBO {
    glm::mat4 model, view, proj;
    MVP_UBO() {
        model = glm::mat4(1.0f);
        view = glm::mat4(1.0f);
        proj = glm::mat4(1.0f);
    }
};