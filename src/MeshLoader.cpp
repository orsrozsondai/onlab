#include "MeshLoader.hpp"
#include <cstddef>
#define TINYOBJLOADER_IMPLEMENTATION
#include "Vertex.hpp"
#include "tiny_obj_loader.h"
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

MeshLoader::MeshLoader(const std::string& path) : path(path) {
    loadOBJ();
}

bool MeshLoader::loadOBJ() {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string err;

    bool success = tinyobj::LoadObj(
        &attrib,
        &shapes,
        nullptr,
        &err,
        path.c_str(),
        nullptr,
        true
    );

    // if (!err.empty()) throw std::runtime_error(err);
    std::cerr << err << std::endl;

    if (!success)
        return false;

    vertices.clear();
    indices.clear();

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            glm::vec3 position{
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            glm::vec3 normal{0.0f};

            if (index.normal_index >= 0)
            {
                normal = glm::vec3(
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                );
            }

            vertices.emplace_back(position, normal, glm::vec3(0.0f));
            indices.push_back(static_cast<uint32_t>(indices.size()));
        }
    }

    computeTangents();

    return true;
}

void MeshLoader::computeTangents() {
    for (size_t i = 0; i < indices.size(); i += 3)
    {
        Vertex& v0 = vertices[indices[i + 0]];
        Vertex& v1 = vertices[indices[i + 1]];
        Vertex& v2 = vertices[indices[i + 2]];

        glm::vec3 edge1 = v1.position - v0.position;
        glm::vec3 edge2 = v2.position - v0.position;

        glm::vec3 tangent = glm::normalize(edge1);

        v0.tangent += tangent;
        v1.tangent += tangent;
        v2.tangent += tangent;
    }

    for (auto& v : vertices)
        v.tangent = glm::normalize(v.tangent);
}

const std::vector<Vertex>& MeshLoader::getVertices() const {
    return vertices;
}

const std::vector<uint32_t>& MeshLoader::getIndices() const {
    return indices;
}