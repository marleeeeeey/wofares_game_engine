#include "weapon_control_system.h"
#include <box2d/b2_math.h>
#include <ecs/components/game_components.h>
#include <my_common_cpp_utils/Logger.h>
#include <my_common_cpp_utils/MathUtils.h>
#include <utils/box2d_entt_contact_listener.h>
#include <utils/box2d_helpers.h>
#include <utils/glm_box2d_conversions.h>

WeaponControlSystem::WeaponControlSystem(entt::registry& registry_, Box2dEnttContactListener& contactListener)
  : registry(registry_), gameState(registry.get<GameState>(registry.view<GameState>().front())),
    contactListener(contactListener)
{
    contactListener.SubscribeContact(
        Box2dEnttContactListener::ContactType::Begin,
        [this](entt::entity entityA, entt::entity entityB)
        {
            for (const auto& entity : {entityA, entityB})
            {
                if (!registry.all_of<ContactExplosionComponent>(entity))
                    continue;

                auto& contactExplosion = registry.get<ContactExplosionComponent>(entity);
                if (contactExplosion.spawnSafeTime <= 0.0f)
                {
                    // Update Box2D object is not allowed in the contact listener. Because Box2D is in simulation
                    // step. So, we need to store entities in the queue and update them in the main loop.
                    explosionEntities.push(entity);
                }
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
            TryToRunExplosionImpactComponent(timerEntity);
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
    UpdateContactExplosionComponentTimer();
    ProcessExplosionEntitiesQueue();
}

void WeaponControlSystem::OnBazookaContactWithTile(entt::entity bazookaEntity, entt::entity tileEntity)
{
    MY_LOG(info, "Bazooka contact with tile");
};

void WeaponControlSystem::TryToRunExplosionImpactComponent(entt::entity explosionEntity)
{
    auto explosionImpact = registry.try_get<ExplosionImpactComponent>(explosionEntity);
    auto physicsInfo = registry.try_get<PhysicsInfo>(explosionEntity);
    if (explosionImpact && physicsInfo)
    {
        const b2Vec2& grenadePhysicsPos = physicsInfo->bodyRAII->GetBody()->GetPosition();
        auto entitiesUnderExploisonImpact = GetPhysicalBodiesNearGrenade(grenadePhysicsPos, explosionImpact->radius);
        ApplyForceToPhysicalBodies(entitiesUnderExploisonImpact, grenadePhysicsPos, explosionImpact->force);
        StartCollisionDisableTimer(entitiesUnderExploisonImpact);
        registry.destroy(explosionEntity);
    }
};
void WeaponControlSystem::ProcessExplosionEntitiesQueue()
{
    while (!explosionEntities.empty())
    {
        auto entity = explosionEntities.front();
        TryToRunExplosionImpactComponent(entity);
        explosionEntities.pop();
    }
};

void WeaponControlSystem::StartCollisionDisableTimer(const std::vector<entt::entity>& physicalEntities)
{
    for (auto& entity : physicalEntities)
    {
        // Exclude players from the list.
        if (registry.any_of<PlayerInfo>(entity))
            continue;

        if (utils::randomTrue(gameState.levelOptions.colisionDisableProbability))
            registry.emplace_or_replace<CollisionDisableTimerComponent>(entity);
    }
};

void WeaponControlSystem::UpdateContactExplosionComponentTimer()
{
    auto contactExplosionsView = registry.view<ContactExplosionComponent>();
    for (auto& entity : contactExplosionsView)
    {
        auto& contactExplosion = contactExplosionsView.get<ContactExplosionComponent>(entity);
        contactExplosion.spawnSafeTime -= deltaTime;
    }
};