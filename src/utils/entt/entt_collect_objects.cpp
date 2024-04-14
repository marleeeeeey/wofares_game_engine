#include "entt_collect_objects.h"
#include <ecs/components/physics_components.h>
#include <ecs/components/player_components.h>
#include <utils/logger.h>
#include <utils/math_utils.h>

EnttCollectObjects::EnttCollectObjects(entt::registry& registry)
  : registry(registry), gameState(registry.get<GameOptions>(registry.view<GameOptions>().front()))
{}

std::vector<entt::entity> EnttCollectObjects::GetPhysicalBodiesInRaduis(
    const b2Vec2& grenadePhysicsPos, float grenadeExplosionRadius, std::optional<b2BodyType> bodyType)
{
    auto viewTargets = registry.view<PhysicsComponent>();
    std::vector<entt::entity> targetsVector = {viewTargets.begin(), viewTargets.end()};
    return GetPhysicalBodiesInRaduis(targetsVector, grenadePhysicsPos, grenadeExplosionRadius, bodyType);
}

std::vector<entt::entity> EnttCollectObjects::GetPhysicalBodiesInRaduis(
    const std::vector<entt::entity>& entities, const b2Vec2& center, float radius, std::optional<b2BodyType> bodyType)
{
    MY_LOG(
        info, "GetPhysicalBodiesInRaduis: center: {}, radius: {}, entitiesCount {}", center, radius, entities.size());

    std::vector<entt::entity> result;

    for (auto& entity : entities)
    {
        auto physicsInfo = registry.get<PhysicsComponent>(entity);
        b2Body* body = physicsInfo.bodyRAII->GetBody();
        const b2Vec2& posPhysics = body->GetPosition();

        if (bodyType && body->GetType() != bodyType.value())
            continue;

        float distance = utils::CaclDistance(center, posPhysics);
        if (distance <= radius)
            result.push_back(entity);
    }

    MY_LOG(info, "GetPhysicalBodiesInRaduis: resultCount {}", result.size());

    return result;
}

std::vector<entt::entity> EnttCollectObjects::ExcludePlayersFromList(const std::vector<entt::entity>& entities)
{
    std::vector<entt::entity> result;
    for (auto& entity : entities)
    {
        if (!registry.any_of<PlayerComponent>(entity))
            result.push_back(entity);
    }
    return result;
}
