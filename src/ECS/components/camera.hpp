#pragma once

#include <entt.hpp>
#include "ECS/components/spatial.hpp"


struct MVPMatrix
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    
    glm::mat4 value;
};

struct cameraSettings
{
    float fov;
    float width;
    float height;
    float nearPlane;
    float farPlane;
};

entt::entity createCamera(entt::registry& registry, const glm::vec3& position = {0.0f, 0.0f, 0.0f}, const glm::vec3& rotation = {0.0f, 0.0f, -1.0f});

const glm::mat4& recalculateMVP(entt::registry& registry, entt::entity camera);


