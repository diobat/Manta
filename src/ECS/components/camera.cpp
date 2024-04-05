#include "ECS/components/camera.hpp"


namespace{
    constexpr float fov = 45.0f;
    constexpr float width = 800.0f;
    constexpr float height = 600.0f;
    constexpr float nearPlane = 0.1f;
    constexpr float farPlane = 100.0f;
    constexpr float translationSpeed = 0.01f;
    constexpr float rotationSpeed = 0.0005f;
}

entt::entity createCamera(entt::registry& registry, const glm::vec3& positionValue, const glm::vec2& rotationValue)
{
    entt::entity entity = registry.create();

    registry.emplace<position>(entity, positionValue);
    registry.emplace<rotation>(entity, rotationValue);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 proj = glm::perspective(fov, width / height, nearPlane, farPlane);

    registry.emplace<MVPMatrix>(entity, model, view, proj);//, proj * view);

    registry.emplace<cameraSettings>(entity, fov, width, height, nearPlane, farPlane, translationSpeed, rotationSpeed);

    registry.emplace<TAG_camera>(entity, false);

    return entity;
}

const MVPMatrix& recalculateMVP(entt::registry& registry, entt::entity camera)
{
    glm::vec3& pos = registry.get<position>(camera).value;
    glm::vec2& rot = registry.get<rotation>(camera).value;

    MVPMatrix& mvp = registry.get<MVPMatrix>(camera);

    glm::vec3 dir = glm::vec3{
    std::cos(rot.x) * std::cos(rot.y),
    std::sin(rot.y),
    -std::sin(rot.x) * std::cos(rot.y)
    };

    mvp.view = glm::lookAt(pos, pos + dir, glm::vec3(0.0f, 1.0f, 0.0f));
    // mvp.value = mvp.projection * mvp.view;

    return mvp;
}

void moveCamera(entt::registry& registry, entt::entity camera, relativeDirections direction)
{
    auto& pos = registry.get<position>(camera).value;
    auto& rot = registry.get<rotation>(camera).value;
    auto& camSet = registry.get<cameraSettings>(camera);

    glm::vec3 dir = glm::vec3{
    std::cos(rot.x) * std::cos(rot.y),
    std::sin(rot.y),
    -std::sin(rot.x) * std::cos(rot.y)
    };

    glm::vec3 right = glm::normalize(glm::cross(dir, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 up = glm::normalize(glm::cross(right, dir));

    switch(direction)
    {
        case relativeDirections::FORWARD:
            pos += dir * camSet.translationSpeed;
            break;
        case relativeDirections::BACKWARD:
            pos -= dir * camSet.translationSpeed;
            break;
        case relativeDirections::LEFT:
            pos -= right * camSet.translationSpeed;
            break;
        case relativeDirections::RIGHT:
            pos += right * camSet.translationSpeed;
            break;
        case relativeDirections::UP:
            pos += up * camSet.translationSpeed;
            break;
        case relativeDirections::DOWN:
            pos -= up * camSet.translationSpeed;
            break;
    }
}

void rotateCamera(entt::registry& registry, entt::entity camera, const glm::vec2& delta)
{
    auto& rot = registry.get<rotation>(camera).value;
    auto& camSet = registry.get<cameraSettings>(camera);

    if(rot.y > 1.57f)
    {
        rot.y = 1.57f;
    }
    else if(rot.y < -1.57f)
    {
        rot.y = -1.57f;
    }

    deltaRotation(registry, camera, {-delta.x * camSet.rotationSpeed, -delta.y * camSet.rotationSpeed});
}