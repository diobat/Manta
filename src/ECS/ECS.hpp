#pragma once

#include <entt/entt.hpp>

class Manta;



class Scene
{
public:
    Scene::Scene(Manta* _core, entt::registry& registry);

    entt::registry& getRegistry();

    // Generates a new entity
    [[nodiscard]] entt::entity newEntity();

    // Model functions
    entt::entity addModel(const std::string& path);

    // Camera functions
    entt::entity addCamera();
    void setActiveCamera(const entt::entity& camera);
    entt::entity getActiveCamera() const;
    void moveActiveCamera(unsigned int direction);

private:

    Manta* _core;
    entt::registry& _registry;
};
