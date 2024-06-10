#include "ECS/components/camera.hpp"


namespace{
    constexpr float fov = 45.0f;
    constexpr float width = 800.0f;
    constexpr float height = 600.0f;
    constexpr float nearPlane = 0.1f;
    constexpr float farPlane = 100.0f;
    constexpr float translationSpeed = 0.1f;
    constexpr float rotationSpeed = 0.0005f;
}

entt::entity createCamera(entt::registry& registry, const glm::vec3& positionValue, const glm::vec2& rotationValue)
{
    entt::entity entity = registry.create();

    registry.emplace<position>(entity, positionValue); 

    // Initialize rotation with euler angles
    glm::quat rot = glm::quat(glm::vec3( rotationValue.x, 0.0f, rotationValue.y));

    registry.emplace<rotation>(entity, rot);

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
    glm::quat& rotQuat = registry.get<rotation>(camera).value;
    glm::vec3 rot = glm::eulerAngles(rotQuat);

    MVPMatrix& mvp = registry.get<MVPMatrix>(camera);

    glm::vec3 dir = glm::vec3{
    std::cos(rot.x) * std::cos(rot.y),
    std::sin(rot.y),
    -std::sin(rot.x) * std::cos(rot.y)
    };

    mvp.view = glm::lookAt(pos, pos + dir, glm::vec3(0.0f, 1.0f, 0.0f));

    return mvp;
}

void moveCamera(entt::registry& registry, entt::entity camera, relativeDirections direction)
{
    auto& pos = registry.get<position>(camera).value;
    auto& rotQuat = registry.get<rotation>(camera).value;
    auto& camSet = registry.get<cameraSettings>(camera);

    // Quaternion to euler angles
    glm::vec3 rot = glm::eulerAngles(rotQuat);

    // Euler angles to direction vector
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

void rotateCamera(entt::registry& registry, entt::entity camera, const glm::vec2& deltaRot)
{
    auto& rot = registry.get<rotation>(camera).value;
    auto& camSet = registry.get<cameraSettings>(camera);

    glm::vec2 delta = deltaRot;

    // // Calculate the current pitch from the quaternion
    // float pitch = glm::pitch(rot);

    // // Limit the pitch
    // float maxPitch = glm::radians(89.0f);
    // if(delta.y > 0.0f && pitch >= maxPitch)
    // {
    //     delta.y = 0.0f;
    // }
    // if(delta.y < 0.0f && pitch <= -maxPitch)
    // {
    //     delta.y = 0.0f;
    // }

    // Limit the pitch
    if(delta.y > 0.0f && glm::eulerAngles(rot).y >= glm::radians(89.0f))
    {
        delta.y = 0.0f;
    }
    if(delta.y < 0.0f && glm::eulerAngles(rot).y <= glm::radians(-89.0f))
    {
        delta.y = 0.0f;
    }

    // vec2 to quaternion
    glm::quat yawQuat = glm::angleAxis(delta.y * camSet.rotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f));   
    glm::quat pitchQuat = glm::angleAxis(delta.x * camSet.rotationSpeed, glm::vec3(1.0f, 0.0f, 0.0f));  

    // Apply the yaw rotation globally
    rot = yawQuat * rot;

    // Apply the pitch rotation locally
    rot = rot * pitchQuat;

    // Normalize the quaternion
    rot = glm::normalize(rot);
}

// A function that converts an euler angle rotation delta to a quaternion delta
glm::quat eulerToQuat(const glm::vec3& euler)
{
    glm::quat q = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));
    q = glm::rotate(q, euler.x, glm::vec3(1.0f, 0.0f, 0.0f));
    q = glm::rotate(q, euler.y, glm::vec3(0.0f, 1.0f, 0.0f));
    q = glm::rotate(q, euler.z, glm::vec3(0.0f, 0.0f, 1.0f));

    return q;
}