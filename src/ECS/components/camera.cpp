#include "ECS/components/camera.hpp"


namespace{
    constexpr float fov = 45.0f;
    constexpr float width = 800.0f;
    constexpr float height = 600.0f;
    constexpr float nearPlane = 0.1f;
    constexpr float farPlane = 100.0f;
}

entt::entity createCamera(entt::registry& registry, const glm::vec3& positionValue, const glm::vec3& rotationValue)
{
    entt::entity entity = registry.create();

    registry.emplace<position>(entity, positionValue);
    registry.emplace<rotation>(entity, glm::normalize(rotationValue));

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(positionValue, positionValue + rotationValue, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 proj = glm::perspective(fov, width / height, nearPlane, farPlane);

    registry.emplace<MVPMatrix>(entity, model, view, proj, proj * view);

    registry.emplace<cameraSettings>(entity, fov, width, height, nearPlane, farPlane);

    return entity;
}

const glm::mat4& recalculateMVP(entt::registry registry, entt::entity camera)
{
    glm::vec3& pos = registry.get<position>(camera).value;
    glm::vec3& rot = registry.get<rotation>(camera).value;

    MVPMatrix& mvp = registry.get<MVPMatrix>(camera);

    mvp.view = glm::lookAt(pos, pos + rot, glm::vec3(0.0f, 1.0f, 0.0f));
    
    mvp.value = mvp.projection * mvp.view;

    return mvp.value;
}