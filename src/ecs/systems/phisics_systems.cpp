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
    auto gameStateEntity = registry.view<GameState>().front();
    const auto& gameState = registry.get<GameState>(gameStateEntity);

    auto view = registry.view<Position, Velocity>();
    for (auto entity : view)
    {
        auto& pos = view.get<Position>(entity).value;
        auto& vel = view.get<Velocity>(entity).value;

        vel.y += gameState.gravity * deltaTime;
        pos += vel * deltaTime;
    }
}
