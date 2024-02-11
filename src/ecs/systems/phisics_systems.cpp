#include "phisics_systems.h"
#include <ecs/components/all_components.h>

void PhysicsSystem(entt::registry& registry, float deltaTime)
{
    // Get the physics world.
    auto& gameState = registry.get<GameState>(registry.view<GameState>().front());
    auto physicsWorld = gameState.physicsWorld;

    int32 velocityIterations = 6; // TODO: think how it impacts the performance.
    int32 positionIterations = 2;

    // Update the physics world.
    physicsWorld->Step(deltaTime, velocityIterations, positionIterations);
}
