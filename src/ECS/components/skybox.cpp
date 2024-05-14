#include "ECS/components/skybox.hpp"

#include "rendering/rendering.hpp"


Skybox createSkybox(std::shared_ptr<rendering_system> core, const std::string& path)
{
    Skybox skybox;

    skybox.cube = shapes::cube::model();
    skybox.skyboxTexture = core->getTextureSystem().bakeCubemap(path, true);

    return skybox;
}