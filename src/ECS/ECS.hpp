#pragma once

#include <entt.hpp>

class ECS
{
public:

    entt::registry& getRegistry();

private:
    entt::registry m_registry;
};
