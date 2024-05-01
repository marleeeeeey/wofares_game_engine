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
    auto view = registry.view<PhysicsComponent, MarkedForTrailDebugComponent>();
    for (auto entity : view)
    {
        auto markedForTrailDebugComponent = view.get<MarkedForTrailDebugComponent>(entity);
        std::string name = MY_FMT("Trail{}", entity);
        ObjectsFactory::DebugSpawnOptions options;
        options.spawnPolicy = ObjectsFactory::SpawnPolicyBase::Last;
        options.trailSize = markedForTrailDebugComponent.trailSize;
        objectsFactory.SpawnDebugVisualObject(entity, name, options);
    }
}
