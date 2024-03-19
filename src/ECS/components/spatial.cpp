#include "ECS/components/spatial.hpp"

void setPosition(entt::registry& registry, entt::entity& entity, const glm::vec3& value)
{
    auto& pos = registry.get<position>(entity);
    pos.value = value;
}

void deltaPosition(entt::registry& registry, entt::entity& entity, const glm::vec3& value)
{
    auto& pos = registry.get<position>(entity);
    pos.value += value;
}

const glm::vec3& getPosition(entt::registry& registry, entt::entity& entity)
{
    return registry.get<position>(entity).value;
}

void setRotation(entt::registry& registry, entt::entity& entity, const glm::vec2& value)
{
    auto& rot = registry.get<rotation>(entity);
    rot.value = value;
}

void deltaRotation(entt::registry& registry, entt::entity& entity, const glm::vec2& value)
{
    auto& rot = registry.get<rotation>(entity);
    rot.value += value;
}

const glm::vec2& getRotation(entt::registry& registry, entt::entity& entity)
{
    return registry.get<rotation>(entity).value;
}