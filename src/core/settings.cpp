#include "core/settings.hpp"

void initializeSettingsData(entt::registry& registry)
{
    settingsData data = {true, 3640, 2000, 2};

    settingsEntity = registry.create();
    registry.emplace<settingsData>(settingsEntity, data);
}


const settingsData& getSettingsData(entt::registry& registry)
{
    return registry.get<settingsData>(settingsEntity);
}