#pragma once
#include <Box2D/Box2D.h>
#include <ecs/components/physics_components.h>
#include <entt/entt.hpp>
#include <optional>

template <typename T>
std::optional<b2Vec2> FindClosestTargetPos(entt::registry& registry, const b2Vec2& anchorPosWorld)
{
    auto targetEntities = registry.view<PhysicsComponent, T>();
    float minDistance = std::numeric_limits<float>::max();
    std::optional<b2Vec2> closestTargetPos;
    for (auto targetEntity : targetEntities)
    {
        auto& targetPhysicsComponent = targetEntities.template get<PhysicsComponent>(targetEntity);
        auto targetPos = targetPhysicsComponent.bodyRAII->GetBody()->GetPosition();
        float distance = b2Distance(targetPos, anchorPosWorld);
        if (distance < minDistance)
        {
            minDistance = distance;
            closestTargetPos = targetPos;
        }
    }
    return closestTargetPos;
}
