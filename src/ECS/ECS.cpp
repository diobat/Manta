#include "ECS/ECS.hpp"

#include "core/Manta.hpp"

#include "rendering/rendering.hpp"
#include "rendering/resources/memory.hpp"
#include "ECS/components/spatial.hpp"
#include "ECS/components/camera.hpp"
#include "core/settings.hpp"

Scene::Scene(Manta* core, entt::registry& registry) : 
    _core(core),
    _registry(registry)
{
    ;
}

entt::registry& Scene::getRegistry()
{
    return _registry;
}

entt::entity Scene::newEntity()
{
    return _registry.create();
}

entt::entity Scene::addCamera()
{
    entt::entity camera = createCamera(_registry);
    
    // Get number of frames in flight
    size_t framesInFlight = getSettingsData(_registry).framesInFlight;

    // Create uniform buffers for MVPMatrix
    memoryBuffers memory;
    memory.buffers = _core->getRendering().lock()->getMemorySystem().createUniformBuffers<MVPMatrix>(framesInFlight);

    // Add buffers to camera
    _registry.emplace<memoryBuffers>(camera).buffers = memory.buffers;

    return camera;
}


void Scene::setActiveCamera(const entt::entity& camera)
{
    auto view = _registry.view<TAG_camera>();

    for(auto entity : view)
    {
        if(entity == camera)
        {
            _registry.emplace_or_replace<TAG_camera>(entity, true);
        }
        else
        {
            _registry.emplace_or_replace<TAG_camera>(entity, false);
        }
    }
}

entt::entity Scene::getActiveCamera() const
{
    auto view = _registry.view<TAG_camera>();

    for(auto entity : view)
    {
        if(_registry.get<TAG_camera>(entity).isActiveCamera)
        {
            return entity;
        }
    }
    return entt::null;
}

void Scene::moveActiveCamera(unsigned int direction)
{
    auto camera = getActiveCamera();
    moveCamera(_registry, camera, static_cast<relativeDirections>(direction));
}