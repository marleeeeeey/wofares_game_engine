#include "phisics_systems.h"
#include <ecs/components/all_components.h>

void PhysicsSystem(entt::registry& registry, float deltaTime)
{
    // Get the physics world.
    auto& gameState = registry.get<GameState>(registry.view<GameState>().front());
    auto physicsWorld = gameState.physicsWorld;

    // Update the physics world.
    physicsWorld->Step(
        deltaTime, gameState.physicsOptions.velocityIterations, gameState.physicsOptions.positionIterations);
}

void RemoveDistantObjectsSystem(entt::registry& registry)
{
    auto& gameState = registry.get<GameState>(registry.view<GameState>().front());
    auto levelBounds = gameState.levelOptions.levelBounds;

    auto physicalBodies = registry.view<PhysicalBody>();
    for (auto entity : physicalBodies)
    {
        auto& physicalBody = physicalBodies.get<PhysicalBody>(entity);
        b2Vec2 pos = physicalBody.value->GetBody()->GetPosition();

        if (pos.x < levelBounds.min.x || pos.x > levelBounds.max.x || pos.y < levelBounds.min.y ||
            pos.y > levelBounds.max.y)
        {
            registry.destroy(entity);
        }
    }
}
