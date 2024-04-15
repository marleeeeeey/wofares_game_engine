#include "game_logic_system.h"
#include "my_cpp_utils/math_utils.h"
#include <ecs/components/physics_components.h>
#include <ecs/components/player_components.h>
#include <ecs/components/portal_components.h>
#include <ecs/components/timer_components.h>
#include <utils/box2d/box2d_glm_conversions.h>
#include <utils/entt/entt_registry_requests.h>
#include <utils/logger.h>

GameLogicSystem::GameLogicSystem(entt::registry& registry) : registry(registry), registryWrapper(registry)
{}

void GameLogicSystem::Update(float deltaTime)
{
    UpdatePortalObjectsPosition(deltaTime);
    MagnetDesctructibleParticlesToPortal(deltaTime);
    DestroyClosestDestructibleParticlesInPortal();
    IfPortalsTooCloseToEachOtherScatterThem();
}

void GameLogicSystem::UpdatePortalObjectsPosition(float deltaTime)
{
    if (deltaTime == 0.0f)
        return;

    auto portalEntities = registry.view<PhysicsComponent, PortalComponent>();
    for (auto entity : portalEntities)
    {
        auto& portalComponent = portalEntities.get<PortalComponent>(entity);
        if (portalComponent.isSleeping)
            continue;

        auto& physicsComponent = portalEntities.get<PhysicsComponent>(entity);
        auto portalBody = physicsComponent.bodyRAII->GetBody();
        auto portalPos = portalBody->GetPosition();

        auto closestTargetPosOpt = FindPortalTargetPos(portalPos);
        if (!closestTargetPosOpt.has_value())
            continue;

        // Apply the force to phiysics body to move it to the closest target
        b2Vec2 direction = closestTargetPosOpt.value() - portalPos;
        direction.Normalize();
        // change object speed to the target speed
        b2Vec2 speed = portalBody->GetLinearVelocity();
        b2Vec2 targetSpeed = direction * portalComponent.speed;
        b2Vec2 speedDiff = targetSpeed - speed;
        b2Vec2 force = speedDiff * portalBody->GetMass() / deltaTime;
        portalBody->ApplyForceToCenter(force, true);
    }
}

std::optional<b2Vec2> GameLogicSystem::FindPortalTargetPos(b2Vec2 portalPos)
{
    auto closestExplosionParticlesPos = FindClosestTargetPos<ExplostionParticlesComponent>(registry, portalPos);
    if (closestExplosionParticlesPos.has_value())
    {
        // If the closest explosion particles are too close to the portal, return this position.
        if (b2Distance(closestExplosionParticlesPos.value(), portalPos) < 3.0f)
            return closestExplosionParticlesPos;
    }

    auto closestStickyPos = FindClosestTargetPos<StickyComponent>(registry, portalPos);
    if (closestStickyPos.has_value())
        return closestStickyPos;

    auto closestPlayerPos = FindClosestTargetPos<PlayerComponent>(registry, portalPos);
    if (closestPlayerPos.has_value())
        return closestPlayerPos;

    return std::nullopt;
}

void GameLogicSystem::MagnetDesctructibleParticlesToPortal(float deltaTime)
{
    auto portalEntities = registry.view<PhysicsComponent, PortalComponent>();
    for (auto entity : portalEntities)
    {
        auto& portalComponent = portalEntities.get<PortalComponent>(entity);
        if (portalComponent.isSleeping)
            continue;

        auto& physicsComponent = portalEntities.get<PhysicsComponent>(entity);
        auto portalBody = physicsComponent.bodyRAII->GetBody();
        auto portalPos = portalBody->GetPosition();

        auto entitiesToMagnet = FindEntitiesInRadius<ExplostionParticlesComponent>(registry, portalPos, 6.0f);

        for (auto entityToMagnet : entitiesToMagnet)
        {
            auto& physicsComponentToMagnet = registry.get<PhysicsComponent>(entityToMagnet);
            auto bodyToMagnet = physicsComponentToMagnet.bodyRAII->GetBody();
            auto bodyPos = bodyToMagnet->GetPosition();
            b2Vec2 direction = portalPos - bodyPos;
            direction.Normalize();
            float distance = b2Distance(portalPos, bodyPos);
            float force = portalComponent.magnetForce * (1.0f / sqrt(distance));
            bodyToMagnet->ApplyForceToCenter(force * direction * deltaTime, true);
        }
    }
}

void GameLogicSystem::DestroyClosestDestructibleParticlesInPortal()
{
    auto portalEntities = registry.view<PhysicsComponent, PortalComponent>();
    for (auto entity : portalEntities)
    {
        auto& physicsComponent = portalEntities.get<PhysicsComponent>(entity);
        auto portalBody = physicsComponent.bodyRAII->GetBody();
        auto portalPos = portalBody->GetPosition();

        auto entitiesInPortal = FindEntitiesInRadius<ExplostionParticlesComponent>(registry, portalPos, 0.1f);
        if (entitiesInPortal.empty())
            continue;

        for (auto entityInPortal : entitiesInPortal)
        {
            registryWrapper.Destroy(entityInPortal);
        }
    }
}

void GameLogicSystem::IfPortalsTooCloseToEachOtherScatterThem()
{
    auto portalEntities = registry.view<PhysicsComponent, PortalComponent>();
    for (auto entity : portalEntities)
    {
        auto& physicsComponent = portalEntities.get<PhysicsComponent>(entity);
        auto portalBody = physicsComponent.bodyRAII->GetBody();
        auto portalPos = portalBody->GetPosition();

        auto mergedPortals = FindEntitiesInRadius<PortalComponent>(registry, portalPos, 0.5f);
        // If there are more than one portal in the same position, scatter them.
        if (mergedPortals.size() > 1)
        {
            // Caclulate the center of the merged portals.
            b2Vec2 mergePortalCenterPos = b2Vec2_zero;
            for (auto mergedPortal : mergedPortals)
            {
                auto& mergedPortalPhysicsComponent = registry.get<PhysicsComponent>(mergedPortal);
                mergePortalCenterPos += mergedPortalPhysicsComponent.bodyRAII->GetBody()->GetPosition();
            }
            mergePortalCenterPos *= 1.0f / mergedPortals.size();

            // Apply the force to phiysics bodies to scatter them.
            for (auto portal : mergedPortals)
            {
                auto& mergedPortalPhysicsComponent = registry.get<PhysicsComponent>(portal);
                auto mergedPortalBody = mergedPortalPhysicsComponent.bodyRAII->GetBody();
                b2Vec2 direction = mergedPortalBody->GetPosition() - mergePortalCenterPos;
                direction.Normalize();
                mergedPortalBody->ApplyForceToCenter(500.0f * direction, true);

                // Sleep the portal for a while.
                auto& portalComponent = registry.get<PortalComponent>(portal);
                portalComponent.isSleeping = true;
                registry.emplace_or_replace<TimerComponent>(
                    portal, utils::Random<float>(0.5f, 1.0f),
                    [this](entt::entity portalEntity)
                    {
                        auto& portalComponent = registry.get<PortalComponent>(portalEntity);
                        portalComponent.isSleeping = false;
                    });
            }
        }
    }
}
