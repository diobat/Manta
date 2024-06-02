#pragma once

#include "rendering/resources/model.hpp"
#include "rendering/resources/texture.hpp"
#include "util/VertexShapes.hpp"

struct Skybox
{
    Model cube;
    image texture;
    bool enabled = false;

    // Optional if PBR is enabled
    image irradianceMap;
    image prefilteredMap;
    image brdfLUT;
};

Skybox createSkybox(std::shared_ptr<rendering_system> core, const std::string& path);