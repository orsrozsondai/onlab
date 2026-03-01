#pragma once

#include <glm/ext/vector_float3.hpp>
struct FragmentUBO {
    glm::vec3 albedo;
    float metallic;
    float roughness;

    glm::vec3 lightPos;
    glm::vec3 lightColor;

    glm::vec3 camPos;

};