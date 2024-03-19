#pragma once

#include <memory>

#include "ECS/ECS.hpp"
#include "rendering/rendering.hpp"
#include "user_input/user_input.hpp"

class Manta
{

public:
    Manta();
    void init();

    void run();

    std::weak_ptr<Scene> getScene() const;
    std::weak_ptr<user_input_system> getUserInput() const;
    std::weak_ptr<rendering_system> getRendering() const;

private:

    std::shared_ptr<Scene> m_scene;                     // ECS scene
    std::shared_ptr<user_input_system> m_user_input;    // User input system
    std::shared_ptr<rendering_system> m_rendering;      // Rendering system

};
