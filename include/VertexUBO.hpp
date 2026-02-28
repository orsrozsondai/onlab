#pragma once
#include <glm/mat4x4.hpp>

struct VertexUBO {
    glm::mat4 model, view, proj;
};