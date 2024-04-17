#include "ECS/components/spatial.hpp"

// Position

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

// Rotation

void setRotation(entt::registry& registry, entt::entity& entity, const glm::quat& value)
{
    auto& rot = registry.get<rotation>(entity);
    rot.value = value;
}

void deltaRotation(entt::registry& registry, entt::entity& entity, const glm::quat& value)
{
    auto& rot = registry.get<rotation>(entity);
    rot.value = value * rot.value ;
}

const glm::quat& getRotation(entt::registry& registry, entt::entity& entity)
{
    return registry.get<rotation>(entity).value;
}

// Scale

void setScale(entt::registry& registry, entt::entity& entity, const glm::vec3& value)
{
    auto& size = registry.get<scale>(entity);
    size.value = value;
}

const glm::vec3& getScale(entt::registry& registry, entt::entity& entity)
{
    return registry.get<scale>(entity).value;
}
