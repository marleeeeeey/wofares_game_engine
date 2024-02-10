#include "phisics_systems.h"
#include <ecs/components/all_components.h>

void BoundarySystem(entt::registry& registry, const glm::vec2& windowSize)
{
    auto view = registry.view<Position, Velocity>();
    for (auto entity : view)
    {
        const auto& [position, velocity] = view.get<Position, Velocity>(entity);

        if (position.value.x < 0)
        {
            position.value.x = 0;
            velocity.value.x = 0;
        }
        else if (position.value.x > windowSize.x)
        {
            position.value.x = windowSize.x;
            velocity.value.x = 0;
        }

        if (position.value.y < 0)
        {
            position.value.y = 0;
            velocity.value.y = 0;
        }
        else if (position.value.y > windowSize.y)
        {
            position.value.y = windowSize.y;
            velocity.value.y = 0;
        }
    }
}
void PhysicsSystem(entt::registry& registry, float deltaTime)
{
    // Get the physics world.
    auto& gameState = registry.get<GameState>(registry.view<GameState>().front());
    auto physicsWorld = gameState.physicsWorld;

    int32 velocityIterations = 6;
    int32 positionIterations = 2;

    // Update the physics world.
    physicsWorld->Step(deltaTime, velocityIterations, positionIterations);

    // Update the position of the entities based on the Box2D bodies.
    auto entities = registry.view<Position, SizeComponent, Box2dObject>();
    for (auto& entity : entities)
    {
        // Get the Box2D body and update the position of the entity.
        b2Body* body = entities.get<Box2dObject>(entity).body->GetBody();
        const b2Vec2& position = body->GetPosition();
        // auto angle = body->GetAngle();

        // Apply the new position to the entity.
        auto& pos = registry.get<Position>(entity);
        pos.value.x = position.x;
        pos.value.y = position.y;
    }
}
