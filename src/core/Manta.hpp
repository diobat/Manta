#pragma once

#include <memory>

class Scene;
class user_input_system;
class rendering_system;
class settingsData;

class Manta
{

public:
    Manta();
    void init();

    void run();

    std::weak_ptr<settingsData> getSettings() const;
    std::weak_ptr<Scene> getScene() const;
    std::weak_ptr<user_input_system> getUserInput() const;
    std::weak_ptr<rendering_system> getRendering() const;

private:

    std::shared_ptr<settingsData> _settings;            // Settings
    std::shared_ptr<Scene> m_scene;                     // ECS scene
    std::shared_ptr<user_input_system> m_user_input;    // User input system
    std::shared_ptr<rendering_system> m_rendering;      // Rendering system

};
