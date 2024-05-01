#pragma once
#include <entt/entity/fwd.hpp>
#include <utils/factories/objects_factory.h>

class DebugSystem
{
    entt::registry& registry;
    ObjectsFactory& objectsFactory;
public:
    DebugSystem(entt::registry& registry, ObjectsFactory& objectsFactory);
    // This method should be called as closer to the end of the frame as possible but before the rendering.
    void Update();
private:
    void SpawnTrailsForMarkedEntities();
};