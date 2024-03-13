
#include "ECS/ECS.hpp"
#include "rendering/rendering.hpp"


int main() {
    rendering_system app;
    ECS scene;


    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}