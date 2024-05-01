#include "debug_system.h"
#include <ecs/components/physics_components.h>
#include <my_cpp_utils/logger.h>
#include <utils/factories/objects_factory.h>

DebugSystem::DebugSystem(entt::registry& registry, ObjectsFactory& objectsFactory)
  : registry(registry), objectsFactory(objectsFactory)
{}

void DebugSystem::Update()
{
    SpawnTrailsForMarkedEntities();
}

void DebugSystem::SpawnTrailsForMarkedEntities()
{
    auto view = registry.view<PhysicsComponent, MarkForTrailDebugComponent>();
    for (auto entity : view)
    {
        auto trailDebugComponent = view.get<MarkForTrailDebugComponent>(entity);
        std::string name = MY_FMT("Trail{}", entity);
        ObjectsFactory::DebugSpawnOptions options;
        options.spawnPolicy = ObjectsFactory::SpawnPolicyBase::First;
        options.trailSize = trailDebugComponent.trailSize;
        if (trailDebugComponent.trailSize == 0)
            options.spawnPolicy = ObjectsFactory::SpawnPolicyBase::All;
        objectsFactory.SpawnDebugVisualObject(entity, name, options);
    }
}
