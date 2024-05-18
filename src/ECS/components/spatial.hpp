#pragma once

#include <entt/entt.hpp>
#include "wrapper/glm.hpp"

#include <glm/gtx/quaternion.hpp>
struct position
{
    glm::vec3 value;
};

void addPositionComponent(entt::registry& registry, entt::entity& entity, const glm::vec3& value);
void setPosition(entt::registry& registry, entt::entity& entity, const glm::vec3& value);
void deltaPosition(entt::registry& registry, entt::entity& entity, const glm::vec3& value);
const glm::vec3& getPosition(entt::registry& registry, entt::entity& entity);


struct rotation
{
    glm::quat value;
};

void addRotationComponent(entt::registry& registry, entt::entity& entity, const glm::quat& value);
void setRotation(entt::registry& registry, entt::entity& entity, const glm::quat& value);
void deltaRotation(entt::registry& registry, entt::entity& entity, const glm::quat& value);
const glm::quat& getRotation(entt::registry& registry, entt::entity& entity);

struct scale
{
    glm::vec3 value;
};

void addScaleComponent(entt::registry& registry, entt::entity& entity, const glm::vec3& value);
void setScale(entt::registry& registry, entt::entity& entity, const glm::vec3& value);
const glm::vec3& getScale(entt::registry& registry, entt::entity& entity);


///// General

void addSpatialComponents(entt::registry& registry, entt::entity entity, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);
