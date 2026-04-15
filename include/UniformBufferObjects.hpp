#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/mat4x4.hpp>
#include <stdexcept>

enum MaterialParameters {
    ALBEDO = 0,
    METALLIC = 1,
    ROUGHNESS = 2,
    SHEEN = 3,
    SHEEN_TINT = 4,
    CLEARCOAT = 5,
    CLEARCOAT_GLOSS = 6
};

struct MaterialUBO {
    alignas(16) glm::vec3 albedo;
    float metallic;
    float roughness;

    float sheen;
    float sheenTint;
    float clearcoat;
    float clearcoatGloss;


    MaterialUBO() {
        albedo = {0,0,0};
        metallic = 0;
        roughness = 0;
        sheen = 0;
        sheenTint = 0;
        clearcoat = 0;
        clearcoatGloss = 0;
    }

    void* get(MaterialParameters param) {
        switch (param) {
            case ALBEDO: return &albedo;
            case METALLIC: return &metallic;
            case ROUGHNESS: return &roughness;
            case SHEEN: return &sheen;
            case SHEEN_TINT: return &sheenTint;
            case CLEARCOAT: return &clearcoat;
            case CLEARCOAT_GLOSS: return &clearcoatGloss;
            default: throw std::invalid_argument({"Invalid parameter: %d", param});
        }
    }

};

struct SceneUBO {
    int ibl;
    alignas(16) glm::vec4 lightPos;
    alignas(16) glm::vec3 lightColor;
    alignas(16) glm::vec3 ambientLight;

    alignas(16) glm::vec3 camPos;
    int toneMapping;
    float exposure;

    SceneUBO() {
        ibl = 1;
        lightPos = {0, -1, 0, 0};
        lightColor = {1, 1, 1};
        ambientLight = {0, 0, 0};
        camPos = {0, 0, 0};
        toneMapping = 1;
        exposure = 1;
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

