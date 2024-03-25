#pragma once

// Third party includes
#include <entt/entt.hpp>

namespace
{
    entt::entity settingsEntity;
}

struct settingsData
{
    const bool enableValidationLayers;

    uint32_t windowWidth;
    uint32_t windowHeight;

    const unsigned int framesInFlight;
};

void initializeSettingsData(entt::registry& registry);

const settingsData& getSettingsData(entt::registry& registry);