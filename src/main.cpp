#include "App.hpp"
#include "Camera.hpp"
#include "Object.hpp"
#include "Pipeline.hpp"
#include "SettingsWindow.hpp"
#include <GLFW/glfw3.h>
#include <exception>
#include <iostream>



int main() {
    App app("vkDemo", glm::vec2(800, 600));
    Pipeline p = Pipeline(app.getRenderContext(), "default.vert","default.frag");

    std::vector<Vertex> cube = {

        // ===== FRONT (+Z) =====
        {{-0.5f,-0.5f, 0.5f}, {0,0,1}, {1,0,0}},
        {{ 0.5f,-0.5f, 0.5f}, {0,0,1}, {1,0,0}},
        {{ 0.5f, 0.5f, 0.5f}, {0,0,1}, {1,0,0}},
        {{-0.5f,-0.5f, 0.5f}, {0,0,1}, {1,0,0}},
        {{ 0.5f, 0.5f, 0.5f}, {0,0,1}, {1,0,0}},
        {{-0.5f, 0.5f, 0.5f}, {0,0,1}, {1,0,0}},

        // ===== BACK (-Z) =====
        {{ 0.5f,-0.5f,-0.5f}, {0,0,-1}, {-1,0,0}},
        {{-0.5f,-0.5f,-0.5f}, {0,0,-1}, {-1,0,0}},
        {{-0.5f, 0.5f,-0.5f}, {0,0,-1}, {-1,0,0}},
        {{ 0.5f,-0.5f,-0.5f}, {0,0,-1}, {-1,0,0}},
        {{-0.5f, 0.5f,-0.5f}, {0,0,-1}, {-1,0,0}},
        {{ 0.5f, 0.5f,-0.5f}, {0,0,-1}, {-1,0,0}},

        // ===== LEFT (-X) =====
        {{-0.5f,-0.5f,-0.5f}, {-1,0,0}, {0,0,1}},
        {{-0.5f,-0.5f, 0.5f}, {-1,0,0}, {0,0,1}},
        {{-0.5f, 0.5f, 0.5f}, {-1,0,0}, {0,0,1}},
        {{-0.5f,-0.5f,-0.5f}, {-1,0,0}, {0,0,1}},
        {{-0.5f, 0.5f, 0.5f}, {-1,0,0}, {0,0,1}},
        {{-0.5f, 0.5f,-0.5f}, {-1,0,0}, {0,0,1}},

        // ===== RIGHT (+X) =====
        {{ 0.5f,-0.5f, 0.5f}, {1,0,0}, {0,0,-1}},
        {{ 0.5f,-0.5f,-0.5f}, {1,0,0}, {0,0,-1}},
        {{ 0.5f, 0.5f,-0.5f}, {1,0,0}, {0,0,-1}},
        {{ 0.5f,-0.5f, 0.5f}, {1,0,0}, {0,0,-1}},
        {{ 0.5f, 0.5f,-0.5f}, {1,0,0}, {0,0,-1}},
        {{ 0.5f, 0.5f, 0.5f}, {1,0,0}, {0,0,-1}},

        // ===== TOP (+Y) =====
        {{-0.5f, 0.5f, 0.5f}, {0,1,0}, {1,0,0}},
        {{ 0.5f, 0.5f, 0.5f}, {0,1,0}, {1,0,0}},
        {{ 0.5f, 0.5f,-0.5f}, {0,1,0}, {1,0,0}},
        {{-0.5f, 0.5f, 0.5f}, {0,1,0}, {1,0,0}},
        {{ 0.5f, 0.5f,-0.5f}, {0,1,0}, {1,0,0}},
        {{-0.5f, 0.5f,-0.5f}, {0,1,0}, {1,0,0}},

        // ===== BOTTOM (-Y) =====
        {{-0.5f,-0.5f,-0.5f}, {0,-1,0}, {1,0,0}},
        {{ 0.5f,-0.5f,-0.5f}, {0,-1,0}, {1,0,0}},
        {{ 0.5f,-0.5f, 0.5f}, {0,-1,0}, {1,0,0}},
        {{-0.5f,-0.5f,-0.5f}, {0,-1,0}, {1,0,0}},
        {{ 0.5f,-0.5f, 0.5f}, {0,-1,0}, {1,0,0}},
        {{-0.5f,-0.5f, 0.5f}, {0,-1,0}, {1,0,0}},
    };

    auto obj = std::make_unique<Object>(app.getRenderContext(), &p, cube);
    Object* ptr = app.addObject(std::move(obj));
    

    Camera camera(
        {0, 0, 0},
        5.0f,
        app.getWindowSize().x/app.getWindowSize().y,
        45.0f
    );
    app.setCamera(&camera);

    SettingsWindow settingsWindow(app.getRenderContext());

    settingsWindow.setControlledObject(ptr);

    app.addSettingsWindow(&settingsWindow);
    // return 0;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return EXIT_SUCCESS;
}
