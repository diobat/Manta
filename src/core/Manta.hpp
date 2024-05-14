#pragma once

#include <entt/entt.hpp>
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

    entt::registry& getRegistry();

    std::weak_ptr<settingsData> getSettings() const;
    std::weak_ptr<Scene> getScene() const;
    std::weak_ptr<user_input_system> getUserInput() const;
    std::weak_ptr<rendering_system> getRendering() const;

    // Load Model
    void loadModel(
        const std::string& path, 
        std::array<float, 3> position = {0.0f, 0.0f, 0.0f}, 
        std::array<float, 3> rotation = {0.0f, 0.0f, 0.0f},
        std::array<float, 3> scale = {1.0f, 1.0f, 1.0f});

    // Load Skybox
    void loadSkybox(const std::string& path, bool setAsActive = true);


private:
    entt::registry _registry;                          // ECS registry

    std::shared_ptr<settingsData> _settings;            // Settings
    std::shared_ptr<Scene> m_scene;                     // ECS scene
    std::shared_ptr<user_input_system> m_user_input;    // User input system
    std::shared_ptr<rendering_system> m_rendering;      // Rendering system

};
