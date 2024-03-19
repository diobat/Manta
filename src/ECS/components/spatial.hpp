#pragma once

#include <entt.hpp>
#include "wrapper/glm.hpp"

struct position
{
    glm::vec3 value;
};

void setPosition(entt::registry& registry, entt::entity& entity, const glm::vec3& value);
void deltaPosition(entt::registry& registry, entt::entity& entity, const glm::vec3& value);
const glm::vec3& getPosition(entt::registry& registry, entt::entity& entity);


struct rotation
{
    glm::vec2 value;
};

void setRotation(entt::registry& registry, entt::entity& entity, const glm::vec2& value);
void deltaRotation(entt::registry& registry, entt::entity& entity, const glm::vec2& value);
const glm::vec2& getRotation(entt::registry& registry, entt::entity& entity);