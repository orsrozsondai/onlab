#include "App.hpp"
#include "Object.hpp"
#include "Pipeline.hpp"
#include <GLFW/glfw3.h>
#include <exception>
#include <iostream>



int main() {
    App app("vkDemo", glm::vec2(800, 600));
    Pipeline p = Pipeline(app.getRenderContext(), "default.vert","default.frag");
    Object o = Object(app.getRenderContext(), &p, {
        Vertex({ -0.5f, -0.5f, 0.0f },{0.0f,1.0f,1.0f}), // bottom left
        Vertex({  0.5f, -0.5f, 0.0f },{1.0f,0.0f,0.0f}), // bottom right
        Vertex({  0.5f,  0.5f, 0.0f },{0.0f,0.0f,1.0f}), // top right
        Vertex({ -0.5f,  0.5f, 0.0f },{0.0f,1.0f,0.0f})  // top left
    });
    
    app.addObject(o);

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return EXIT_SUCCESS;
}
