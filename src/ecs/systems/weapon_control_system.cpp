#include "weapon_control_system.h"
#include "box2d/b2_math.h"
#include "utils/box2d_entt_contact_listener.h"
#include <ecs/components/game_components.h>
#include <utils/glm_box2d_conversions.h>

WeaponControlSystem::WeaponControlSystem(entt::registry& registry, Box2dEnttContactListener& contactListener)
  : registry(registry), gameState(registry.get<GameState>(registry.view<GameState>().front())),
    contactListener(contactListener)
{}

void WeaponControlSystem::ImpactTargetsInGrenadesExplosionRadius()
{
    auto viewGrenedes = registry.view<Grenade, PhysicalBody>();
    for (auto& grenadeEntity : viewGrenedes)
    {
        auto& grenade = viewGrenedes.get<Grenade>(grenadeEntity);
        auto& grenadeBody = viewGrenedes.get<PhysicalBody>(grenadeEntity);
        const b2Vec2& grenadePhysicsPos = grenadeBody.value->GetBody()->GetPosition();
        grenade.timeToExplode -= deltaTime;

        if (grenade.timeToExplode <= 0.0f)
        {
            auto physicalBodiesNearGrenade = GetPhysicalBodiesNearGrenade(grenadePhysicsPos, grenade.explosionRadius);
            ApplyForceToPhysicalBodies(physicalBodiesNearGrenade, grenadePhysicsPos);
            // Warning: double destroy of the grenade entity.
            registry.destroy(grenadeEntity);
        }
    }
}

std::vector<entt::entity> WeaponControlSystem::GetPhysicalBodiesNearGrenade(
    const b2Vec2& grenadePhysicsPos, float grenadeExplosionRadius)
{
    std::vector<entt::entity> physicalBodiesNearGrenade;
    auto viewTargets = registry.view<PhysicalBody>();
    for (auto& targetEntity : viewTargets)
    {
        auto& targetBody = viewTargets.get<PhysicalBody>(targetEntity);

        // Calculate distance between grenade and target.
        const auto& targetPhysicsPos = targetBody.value->GetBody()->GetPosition();
        float distance = utils::distance(grenadePhysicsPos, targetPhysicsPos);

        if (distance <= grenadeExplosionRadius)
            physicalBodiesNearGrenade.push_back(targetEntity);
    }
    return physicalBodiesNearGrenade;
}

void WeaponControlSystem::ApplyForceToPhysicalBodies(
    std::vector<entt::entity> physicalEntities, const b2Vec2& grenadePhysicsPos)
{
    for (auto& entity : physicalEntities)
    {
        auto targetBody = registry.get<PhysicalBody>(entity).value->GetBody();
        const auto& targetPhysicsPos = targetBody->GetPosition();

        // Make target body as dynamic.
        targetBody->SetType(b2_dynamicBody);

        // Calculate distance between grenade and target.
        float distance = utils::distance(grenadePhysicsPos, targetPhysicsPos);

        // Apply force to the target.
        // Force direction is from grenade to target. Inside. This greate interesting effect.
        auto force = -(targetPhysicsPos - grenadePhysicsPos) * 1000.0f;
        targetBody->ApplyForceToCenter(force, true);
    }
}

void WeaponControlSystem::Update(float deltaTime)
{
    this->deltaTime = deltaTime;
    ImpactTargetsInGrenadesExplosionRadius();
}
