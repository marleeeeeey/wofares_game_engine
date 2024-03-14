#include "collect_objects.h"
#include <utils/math_utils.h>

CollectObjects::CollectObjects(entt::registry& registry, ObjectsFactory& objectsFactory)
  : registry(registry), gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())),
    objectsFactory(objectsFactory)
{}

std::vector<entt::entity> CollectObjects::GetPhysicalBodiesInRaduis(
    const b2Vec2& grenadePhysicsPos, float grenadeExplosionRadius, std::optional<b2BodyType> bodyType)
{
    auto viewTargets = registry.view<PhysicsInfo>();
    std::vector<entt::entity> targetsVector = {viewTargets.begin(), viewTargets.end()};
    return GetPhysicalBodiesInRaduis(targetsVector, grenadePhysicsPos, grenadeExplosionRadius, bodyType);
}

std::vector<entt::entity> CollectObjects::GetPhysicalBodiesInRaduis(
    const std::vector<entt::entity>& entities, const b2Vec2& center, float radius, std::optional<b2BodyType> bodyType)
{
    std::vector<entt::entity> result;

    for (auto& entity : entities)
    {
        auto physicsInfo = registry.get<PhysicsInfo>(entity);
        b2Body* body = physicsInfo.bodyRAII->GetBody();
        const b2Vec2& physicsPos = body->GetPosition();

        if (bodyType && body->GetType() != bodyType.value())
            continue;

        float distance = utils::CaclDistance(center, physicsPos);
        if (distance <= radius)
            result.push_back(entity);
    }

    return result;
};

std::vector<entt::entity> CollectObjects::ExcludePlayersFromList(const std::vector<entt::entity>& entities)
{
    std::vector<entt::entity> result;
    for (auto& entity : entities)
    {
        if (!registry.any_of<PlayerInfo>(entity))
            result.push_back(entity);
    }
    return result;
};
