#include "phisics_systems.h"
#include <ecs/components/all_components.h>
#include <utils/glm_box2d_conversions.h>

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
    auto levelBounds = gameState.levelOptions.levelBox2dBounds;

    auto physicalBodies = registry.view<PhysicalBody>();
    for (auto entity : physicalBodies)
    {
        auto& physicalBody = physicalBodies.get<PhysicalBody>(entity);
        b2Vec2 pos = physicalBody.value->GetBody()->GetPosition();

        if (!IsPointInsideBounds(pos, levelBounds))
        {
            registry.destroy(entity);
        }
    }
}
