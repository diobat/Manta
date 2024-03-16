
#include <memory>

#include "ECS/ECS.hpp"
#include "rendering/rendering.hpp"


int main() {
    std::shared_ptr<Scene> scene = std::make_shared<Scene>();

    scene->addCamera();


    rendering_system app;
    app.setScene(scene);

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}