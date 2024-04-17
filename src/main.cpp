
#include <memory>
#include <iostream>

#include "core/Manta.hpp"

int main() {
    Manta engine;

    engine.loadModel("res/models/room/viking_room.obj", {3.0f, 3.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f});
    engine.loadModel("res/models/room/viking_room.obj", {-3.0f, 3.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f});
    engine.loadModel("res/models/room/viking_room.obj", {0.0f, -3.0f, 3.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f});


    try {
        engine.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}