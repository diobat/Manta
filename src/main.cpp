
#include <memory>

#include "core/Manta.hpp"


int main() {
    Manta engine;

    try {
        engine.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}