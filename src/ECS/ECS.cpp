#include "ECS/ECS.hpp"

entt::registry& ECS::getRegistry()
{
    return m_registry;
}