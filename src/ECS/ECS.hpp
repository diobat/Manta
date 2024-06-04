#pragma once

#include <entt/entt.hpp>

#include <vector>
#include <string>

class Manta;

class Scene
{
public:
    Scene(Manta* _core, entt::registry& registry);

    entt::registry& getRegistry();

    // Generates a new entity
    [[nodiscard]] entt::entity newEntity();

    // Model functions
    entt::entity addModel(const std::string& path, const std::array<float, 3>& position = {0.0f, 0.0f, 0.0f}, const std::array<float, 3>& rotation = {0.0f, 0.0f, 0.0f}, const std::array<float, 3>& scale = {1.0f, 1.0f, 1.0f});
    std::vector<std::string> getAllModelNames() const;

    // Camera functions
    entt::entity addCamera();
    void setActiveCamera(const entt::entity& camera);
    entt::entity getActiveCamera() const;
    void moveActiveCamera(unsigned int direction);

    // Skybox functions
    entt::entity addSkybox(const std::string& path, bool setActive = false);
    void setActiveSkybox(const entt::entity& skybox);

private:

    Manta* _core;
    entt::registry& _registry;
};
