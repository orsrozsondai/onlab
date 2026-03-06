#pragma once

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
struct FragmentUBO {
    alignas(16) glm::vec3 albedo;
    float metallic;
    float roughness;

    alignas(16) glm::vec4 lightPos;
    alignas(16) glm::vec3 lightColor;

    alignas(16) glm::vec3 camPos;

    FragmentUBO() {
        albedo = {0,0,0};
        metallic = 0;
        roughness = 0;
        lightPos = {0,0,0,0};
        lightColor = {1,1,1};
        camPos = {0,0,0};
    }

};