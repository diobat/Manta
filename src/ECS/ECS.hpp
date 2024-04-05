#pragma once

#include <entt/entt.hpp>

class Manta;

class Scene
{
public:
    Scene::Scene(Manta* _core, entt::registry& registry);

    entt::registry& getRegistry();

    entt::entity addCamera();

    void setActiveCamera(const entt::entity& camera);
    entt::entity getActiveCamera() const;
    void moveActiveCamera(unsigned int direction);

private:

    Manta* _core;
    entt::registry& _registry;
};
