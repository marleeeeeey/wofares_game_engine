#include "phisics_systems.h"
#include <ecs/components/all_components.h>

void PhysicsSystem(entt::registry& registry, float deltaTime)
{
    // Get the physics world.
    auto& gameState = registry.get<GameState>(registry.view<GameState>().front());
    auto physicsWorld = gameState.physicsWorld;

    // Update the physics world.
    physicsWorld->Step(deltaTime, gameState.velocityIterations, gameState.positionIterations);
}
