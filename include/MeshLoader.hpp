#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "Vertex.hpp"

class MeshLoader {
public:

    MeshLoader(const std::string& path);

    const std::vector<Vertex>& getVertices() const;
    const std::vector<uint32_t>& getIndices() const;

private:
    bool loadOBJ();
    void computeTangents();

    std::string path;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};