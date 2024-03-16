#pragma once

#include <entt.hpp>

class Scene
{
public:
    entt::registry& getRegistry();

    entt::entity addCamera();

private:
    entt::registry m_registry;
};
