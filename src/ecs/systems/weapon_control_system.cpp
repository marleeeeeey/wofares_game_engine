#include "weapon_control_system.h"
#include "box2d/b2_math.h"
#include "utils/box2d_entt_contact_listener.h"
#include <ecs/components/game_components.h>
#include <my_common_cpp_utils/Logger.h>
#include <utils/glm_box2d_conversions.h>

WeaponControlSystem::WeaponControlSystem(entt::registry& registry_, Box2dEnttContactListener& contactListener)
  : registry(registry_), gameState(registry.get<GameState>(registry.view<GameState>().front())),
    contactListener(contactListener)
{
    contactListener.SubscribeContact(
        Box2dEnttContactListener::ContactType::Begin,
        [this](entt::entity entityA, entt::entity entityB)
        {
            bool isEntityABazooka = registry.all_of<ContactExplosionComponent>(entityA);
            bool isEntityBBazooka = registry.all_of<ContactExplosionComponent>(entityB);
            if (isEntityABazooka && isEntityBBazooka)
                return;

            auto entityBazooka = isEntityABazooka ? entityA : entityB;
            auto entityTarget = isEntityABazooka ? entityB : entityA;

            if (registry.all_of<RenderingInfo>(entityTarget))
            {
                OnBazookaContactWithTile(entityBazooka, entityTarget);
            }
        });
}

void WeaponControlSystem::UpdateTimerExplosionComponents()
{
    auto timersView = registry.view<TimerExplosionComponent>();
    for (auto& timerEntity : timersView)
    {
        auto& timerExplosion = timersView.get<TimerExplosionComponent>(timerEntity);
        timerExplosion.timeToExplode -= deltaTime;

        if (timerExplosion.timeToExplode <= 0.0f)
        {
            // try to get explosion impact component and physics info.
            auto explosionImpact = registry.try_get<ExplosionImpactComponent>(timerEntity);
            auto physicsInfo = registry.try_get<PhysicsInfo>(timerEntity);
            if (explosionImpact && physicsInfo)
            {
                const b2Vec2& grenadePhysicsPos = physicsInfo->bodyRAII->GetBody()->GetPosition();
                auto physicalBodiesNearGrenade =
                    GetPhysicalBodiesNearGrenade(grenadePhysicsPos, explosionImpact->radius);
                ApplyForceToPhysicalBodies(physicalBodiesNearGrenade, grenadePhysicsPos, explosionImpact->force);
            }

            registry.destroy(timerEntity); // Удаление сущности
        }
    }
}

std::vector<entt::entity> WeaponControlSystem::GetPhysicalBodiesNearGrenade(
    const b2Vec2& grenadePhysicsPos, float grenadeExplosionRadius)
{
    std::vector<entt::entity> physicalBodiesNearGrenade;
    auto viewTargets = registry.view<PhysicsInfo>();
    for (auto& targetEntity : viewTargets)
    {
        auto& targetBody = viewTargets.get<PhysicsInfo>(targetEntity);

        // Calculate distance between grenade and target.
        const auto& targetPhysicsPos = targetBody.bodyRAII->GetBody()->GetPosition();
        float distance = utils::distance(grenadePhysicsPos, targetPhysicsPos);

        if (distance <= grenadeExplosionRadius)
            physicalBodiesNearGrenade.push_back(targetEntity);
    }
    return physicalBodiesNearGrenade;
}

void WeaponControlSystem::ApplyForceToPhysicalBodies(
    std::vector<entt::entity> physicalEntities, const b2Vec2& grenadePhysicsPos, float force)
{
    for (auto& entity : physicalEntities)
    {
        auto targetBody = registry.get<PhysicsInfo>(entity).bodyRAII->GetBody();
        const auto& targetPhysicsPos = targetBody->GetPosition();

        // Make target body as dynamic.
        targetBody->SetType(b2_dynamicBody);

        // Calculate distance between grenade and target.
        float distance = utils::distance(grenadePhysicsPos, targetPhysicsPos);

        // Apply force to the target.
        // Force direction is from grenade to target. Inside. This greate interesting effect.
        auto forceVec = -(targetPhysicsPos - grenadePhysicsPos) * force;
        targetBody->ApplyForceToCenter(forceVec, true);
    }
}

void WeaponControlSystem::Update(float deltaTime)
{
    this->deltaTime = deltaTime;
    UpdateTimerExplosionComponents();
}

void WeaponControlSystem::OnBazookaContactWithTile(entt::entity bazookaEntity, entt::entity tileEntity)
{
    MY_LOG(info, "Bazooka contact with tile");
};