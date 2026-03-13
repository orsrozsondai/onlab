#include "App.hpp"
#include "Camera.hpp"
#include "MeshLoader.hpp"
#include "Object.hpp"
#include "Pipeline.hpp"
#include "Scene.hpp"
#include "SettingsWindow.hpp"
#include "imgui.h"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <sys/types.h>
#include <utility>
#include <vulkan/vulkan_core.h>



int main() {
    std::unique_ptr<MeshLoader> skullMesh;
    skullMesh = std::make_unique<MeshLoader>(MeshLoader("res/models/skull.obj", "Skull"));
    

    // std::cout << "mesh loaded" << std::endl;
    
    
    App app("PBR");

    // std::cout << "app made" << std::endl;
    Pipeline p = Pipeline(app.getRenderContext(), "default.vert","default.frag");

    
    std::vector<Vertex> cubeVertices = {

        // ===== FRONT (+Z) =====
        {{-0.5f,-0.5f, 0.5f}, {0,0,1}, {1,0,0}}, // 0
        {{ 0.5f,-0.5f, 0.5f}, {0,0,1}, {1,0,0}}, // 1
        {{ 0.5f, 0.5f, 0.5f}, {0,0,1}, {1,0,0}}, // 2
        {{-0.5f, 0.5f, 0.5f}, {0,0,1}, {1,0,0}}, // 3

        // ===== BACK (-Z) =====
        {{ 0.5f,-0.5f,-0.5f}, {0,0,-1}, {-1,0,0}}, // 4
        {{-0.5f,-0.5f,-0.5f}, {0,0,-1}, {-1,0,0}}, // 5
        {{-0.5f, 0.5f,-0.5f}, {0,0,-1}, {-1,0,0}}, // 6
        {{ 0.5f, 0.5f,-0.5f}, {0,0,-1}, {-1,0,0}}, // 7

        // ===== LEFT (-X) =====
        {{-0.5f,-0.5f,-0.5f}, {-1,0,0}, {0,0,1}}, // 8
        {{-0.5f,-0.5f, 0.5f}, {-1,0,0}, {0,0,1}}, // 9
        {{-0.5f, 0.5f, 0.5f}, {-1,0,0}, {0,0,1}}, // 10
        {{-0.5f, 0.5f,-0.5f}, {-1,0,0}, {0,0,1}}, // 11

        // ===== RIGHT (+X) =====
        {{ 0.5f,-0.5f, 0.5f}, {1,0,0}, {0,0,-1}}, // 12
        {{ 0.5f,-0.5f,-0.5f}, {1,0,0}, {0,0,-1}}, // 13
        {{ 0.5f, 0.5f,-0.5f}, {1,0,0}, {0,0,-1}}, // 14
        {{ 0.5f, 0.5f, 0.5f}, {1,0,0}, {0,0,-1}}, // 15

        // ===== TOP (+Y) =====
        {{-0.5f, 0.5f, 0.5f}, {0,1,0}, {1,0,0}}, // 16
        {{ 0.5f, 0.5f, 0.5f}, {0,1,0}, {1,0,0}}, // 17
        {{ 0.5f, 0.5f,-0.5f}, {0,1,0}, {1,0,0}}, // 18
        {{-0.5f, 0.5f,-0.5f}, {0,1,0}, {1,0,0}}, // 19

        // ===== BOTTOM (-Y) =====
        {{-0.5f,-0.5f,-0.5f}, {0,-1,0}, {1,0,0}}, // 20
        {{ 0.5f,-0.5f,-0.5f}, {0,-1,0}, {1,0,0}}, // 21
        {{ 0.5f,-0.5f, 0.5f}, {0,-1,0}, {1,0,0}}, // 22
        {{-0.5f,-0.5f, 0.5f}, {0,-1,0}, {1,0,0}}, // 23
    };

    std::vector<uint32_t> cubeIndices = {

        // Front
        0,1,2,
        0,2,3,

        // Back
        4,5,6,
        4,6,7,

        // Left
        8,9,10,
        8,10,11,

        // Right
        12,13,14,
        12,14,15,

        // Top
        16,17,18,
        16,18,19,

        // Bottom
        20,21,22,
        20,22,23
    };

    auto cubeMesh = std::make_unique<MeshLoader>(cubeVertices, cubeIndices, "Cube");

    Object* obj;
    int width, height;
    glfwGetWindowSize(app.getRenderContext().window, &width, &height);
    Camera camera(
        {0, 0, 0},
        5.0f,
        (float)width/height,
        45.0f
    );
    // std::cout << "camera made" << std::endl;
    Scene scene(app.getRenderContext(), &p, &camera);
    // std::cout << "scene made" << std::endl;
    scene.addMesh(std::move(cubeMesh));

    
    // std::cout << "mesh added" << std::endl;
    app.setScene(&scene);
    
    // std::cout << "scene set" << std::endl;
    
    app.setCamera(&camera);
        // std::cout << "running" << std::endl;
        app.run();
   
    app.destroy();
    return EXIT_SUCCESS;
}
