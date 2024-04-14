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

template <typename T>
std::vector<entt::entity> FindEntitiesInRadius(
    entt::registry& registry, const b2Vec2& centerPosPhysics, float radiusPhysics)
{
    std::vector<entt::entity> entitiesInRadius;
    auto entities = registry.view<PhysicsComponent, T>();
    for (auto entity : entities)
    {
        auto& physicsComponent = entities.template get<PhysicsComponent>(entity);
        auto entityPos = physicsComponent.bodyRAII->GetBody()->GetPosition();
        if (b2Distance(centerPosPhysics, entityPos) < radiusPhysics)
            entitiesInRadius.push_back(entity);
    }
    return entitiesInRadius;
}

inline std::vector<entt::entity> FindEntitiesInRadius(
    entt::registry& registry, const b2Vec2& centerPosPhysics, float radiusPhysics)
{
    std::vector<entt::entity> entitiesInRadius;
    auto entities = registry.view<PhysicsComponent>();
    for (auto entity : entities)
    {
        auto& physicsComponent = entities.template get<PhysicsComponent>(entity);
        auto entityPos = physicsComponent.bodyRAII->GetBody()->GetPosition();
        if (b2Distance(centerPosPhysics, entityPos) < radiusPhysics)
            entitiesInRadius.push_back(entity);
    }
    return entitiesInRadius;
}

inline std::vector<entt::entity> FindEntitiesInRadius(
    entt::registry& registry, const std::vector<entt::entity>& entities, const b2Vec2& centerPosPhysics,
    float radiusPhysics)
{
    std::vector<entt::entity> entitiesInRadius;
    for (auto entity : entities)
    {
        auto& physicsComponent = registry.get<PhysicsComponent>(entity);
        auto entityPos = physicsComponent.bodyRAII->GetBody()->GetPosition();
        if (b2Distance(centerPosPhysics, entityPos) < radiusPhysics)
            entitiesInRadius.push_back(entity);
    }
    return entitiesInRadius;
}

template <typename T>
std::vector<entt::entity> RemoveEntitiesWithComponent(
    entt::registry& registry, const std::vector<entt::entity>& entities)
{
    std::vector<entt::entity> result;
    for (auto& entity : entities)
    {
        if (!registry.any_of<T>(entity))
            result.push_back(entity);
    }
    return result;
}

template <typename T>
std::vector<entt::entity> GetEntitiesWithComponent(entt::registry& registry, const std::vector<entt::entity>& entities)
{
    std::vector<entt::entity> result;
    for (auto& entity : entities)
    {
        if (registry.any_of<T>(entity))
            result.push_back(entity);
    }
    return result;
}