#pragma once
#include "utils/factories/base_objects_factory.h"
#include <entt/entity/fwd.hpp>
#include <utils/factories/game_objects_factory.h>

class DebugSystem
{
    entt::registry& registry;
    BaseObjectsFactory& baseObjectsFactory;
public:
    DebugSystem(entt::registry& registry, BaseObjectsFactory& baseObjectsFactory);
    // This method should be called as closer to the end of the frame as possible but before the rendering.
    void Update();
private:
    void SpawnTrailsForMarkedEntities();
};