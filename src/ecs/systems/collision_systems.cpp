#include "collision_systems.h"
#include <ecs/components/all_components.h>
#include <utils/collisions.h>

void CollisionSystem(entt::registry& registry)
{
    auto players = registry.view<Position, SizeComponent, Velocity, PlayerNumber>();
    auto objects = registry.view<Position, SizeComponent>();

    for (auto player : players)
    {
        const auto& [playerPos, playerSize, playerVel] = players.get<Position, SizeComponent, Velocity>(player);
        AABB playerAABB = СalculateAABB(playerPos, playerSize);

        for (auto object : objects)
        {
            if (player == object)
                continue;

            const auto& [objectPos, objectSize] = players.get<Position, SizeComponent>(object);
            AABB objectAABB = СalculateAABB(objectPos, objectSize);

            if (CheckAABBCollision(playerAABB, objectAABB))
            {
                glm::vec2 direction = objectPos.value - playerPos.value;
                float overlapX = 0.5f * (playerAABB.max.x - playerAABB.min.x + objectAABB.max.x - objectAABB.min.x) -
                    std::abs(direction.x);
                float overlapY = 0.5f * (playerAABB.max.y - playerAABB.min.y + objectAABB.max.y - objectAABB.min.y) -
                    std::abs(direction.y);

                if (overlapX < overlapY)
                {
                    playerPos.value.x -= overlapX * glm::sign(playerVel.value.x);
                    playerVel.value.x = 0;
                }
                else
                {
                    playerPos.value.y -= overlapY * glm::sign(playerVel.value.y);
                    playerVel.value.y = 0;
                }
            }
        }
    }
}
