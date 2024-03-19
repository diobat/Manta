#pragma once

#include <entt.hpp>
#include "ECS/components/spatial.hpp"

struct TAG_camera
{
    bool isActiveCamera = false;
};

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
    float translationSpeed;
    float rotationSpeed;
};

entt::entity createCamera(entt::registry& registry, const glm::vec3& position = {0.0f, 0.0f, 1.0f}, const glm::vec3& rotation = {0.0f, 0.0f, -1.0f});

const MVPMatrix& recalculateMVP(entt::registry& registry, entt::entity camera);


enum class relativeDirections : unsigned int
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

void moveCamera(entt::registry& registry, entt::entity camera, relativeDirections direction);
void rotateCamera(entt::registry& registry, entt::entity camera, const glm::vec2& delta);