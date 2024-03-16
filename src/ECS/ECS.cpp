#include "ECS/ECS.hpp"

#include "ECS/components/spatial.hpp"

entt::entity Scene::addCamera()
{
    entt::entity entity = m_registry.create();

    m_registry.emplace<position>(entity, glm::vec3(0.0f, 0.0f, 5.0f));
    m_registry.emplace<rotation>(entity, glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f)));

    return entity;
}

entt::registry& Scene::getRegistry()
{
    return m_registry;
}
//

