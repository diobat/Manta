#pragma once

#include <entt/entt.hpp>

class Scene
{
public:
    entt::registry& getRegistry();

    entt::entity addCamera();

    void setActiveCamera(const entt::entity& camera);
    entt::entity getActiveCamera() const;
    void moveActiveCamera(unsigned int direction);

private:
    entt::registry m_registry;
};
