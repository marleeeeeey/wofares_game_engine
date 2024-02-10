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

    // Update the position of the entities based on the Box2D bodies.
    auto entities = registry.view<Angle, Position, PhysicalBody>();
    for (auto& entity : entities)
    {
        const auto& [targetAngle, targetPosition, physicalBody] = entities.get<Angle, Position, PhysicalBody>(entity);

        // Get the Box2D body.
        b2Body* body = physicalBody.body->GetBody();

        // Apply the new position to the entity.
        const b2Vec2& position = body->GetPosition();
        targetPosition.value.x = position.x;
        targetPosition.value.y = position.y;

        // Apply the new angle to the entity.
        auto angle = body->GetAngle();
        targetAngle.value = angle;
    }
}
