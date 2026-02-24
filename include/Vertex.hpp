#pragma once

#include <glm/ext/vector_float3.hpp>
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    Vertex(const glm::vec3& position, const glm::vec3& normal) : position(position), normal(normal) {}
};