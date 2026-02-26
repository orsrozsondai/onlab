#pragma once

#include <glm/ext/vector_float3.hpp>
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& tangent) : position(position), normal(normal), tangent(tangent) {}
};