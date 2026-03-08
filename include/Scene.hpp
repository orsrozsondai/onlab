#pragma once
#include <glm/ext/vector_float3.hpp>
#include <vector>
#include <memory>
#include "Object.hpp"

class Scene {
private:

    std::vector<std::unique_ptr<Object>> objects;
    glm::vec4 lightPos;
    glm::vec3 lightColor;

public:
    Scene();


    Object* addObject(std::unique_ptr<Object> obj);
};