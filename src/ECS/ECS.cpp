#include "ECS/ECS.hpp"

#include "ECS/components/spatial.hpp"
#include "ECS/components/camera.hpp"

entt::registry& Scene::getRegistry()
{
    return m_registry;
}


entt::entity Scene::addCamera()
{
    return createCamera(m_registry);
}


void Scene::setActiveCamera(const entt::entity& camera)
{
    auto view = m_registry.view<TAG_camera>();

    for(auto entity : view)
    {
        if(entity == camera)
        {
            m_registry.emplace_or_replace<TAG_camera>(entity, true);
        }
        else
        {
            m_registry.emplace_or_replace<TAG_camera>(entity, false);
        }
    }
}

entt::entity Scene::getActiveCamera() const
{
    auto view = m_registry.view<TAG_camera>();

    for(auto entity : view)
    {
        if(m_registry.get<TAG_camera>(entity).isActiveCamera)
        {
            return entity;
        }
    }
    return entt::null;
}

void Scene::moveActiveCamera(unsigned int direction)
{
    auto camera = getActiveCamera();
    moveCamera(m_registry, camera, static_cast<relativeDirections>(direction));
}