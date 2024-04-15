#include "game_logic_system.h"
#include <ecs/components/physics_components.h>
#include <ecs/components/player_components.h>
#include <ecs/components/portal_components.h>
#include <ecs/components/timer_components.h>
#include <entt/entity/fwd.hpp>
#include <my_cpp_utils/config.h>
#include <my_cpp_utils/math_utils.h>
#include <utils/box2d/box2d_glm_conversions.h>
#include <utils/entt/entt_registry_requests.h>
#include <utils/logger.h>

GameLogicSystem::GameLogicSystem(entt::registry& registry, ObjectsFactory& objectsFactory)
  : registry(registry), registryWrapper(registry), bodyTuner(registry), objectsFactory(objectsFactory),
    coordinatesTransformer(registry)
{}

void GameLogicSystem::Update(float deltaTime)
{
    UpdatePortalsPosition(deltaTime);
    MagnetDesctructibleParticlesToPortal(deltaTime);
    DestroyClosestDestructibleParticlesInPortal();
    ScatterPortalsIsTheyCloseToEachOther();
    EatThePlayerByPortalIfCloser();
}

void GameLogicSystem::UpdatePortalsPosition(float deltaTime)
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
    auto closestExplosionParticlesPos =
        request::FindClosestEntityPosWithAllComponents<ExplostionParticlesComponent>(registry, portalPos);
    if (closestExplosionParticlesPos.has_value())
    {
        // If the closest explosion particles are too close to the portal, return this position.
        if (b2Distance(closestExplosionParticlesPos.value(), portalPos) < 3.0f)
            return closestExplosionParticlesPos;
    }

    auto closestStickyPos = request::FindClosestEntityPosWithAllComponents<StickyComponent>(registry, portalPos);
    if (closestStickyPos.has_value())
        return closestStickyPos;

    auto closestPlayer = request::FindClosestEntityWithAllComponents<PlayerComponent>(
        registry, portalPos,
        [this](auto entity)
        {
            // Check if the player is enabled.
            auto& physicsComponent = registry.get<PhysicsComponent>(entity);
            auto body = physicsComponent.bodyRAII->GetBody();
            return body->IsEnabled();
        });
    if (closestPlayer.has_value())
        return registry.get<PhysicsComponent>(closestPlayer.value()).bodyRAII->GetBody()->GetPosition();

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

        auto entitiesToMagnet =
            request::FindEntitiesWithAllComponentsInRadius<ExplostionParticlesComponent>(registry, portalPos, 6.0f);

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

    portalEntities.each(
        [this](
            [[maybe_unused]] entt::entity portalEntity, PhysicsComponent& portalPhysics,
            PortalComponent& portalComponent)
        {
            if (portalComponent.isSleeping)
                return;

            auto portalBody = portalPhysics.bodyRAII->GetBody();
            auto portalPos = portalBody->GetPosition();

            auto entitiesInPortal =
                request::FindEntitiesWithAllComponentsInRadius<ExplostionParticlesComponent>(registry, portalPos, 0.1f);
            if (entitiesInPortal.empty())
                return;

            for (auto entityInPortal : entitiesInPortal)
            {
                portalComponent.foodCounter++;

                const auto& portalMaxFoodCounter = utils::GetConfig<size_t, "GameLogicSystem.portalMaxFoodCounter">();
                if (portalComponent.foodCounter >= portalMaxFoodCounter)
                {
                    MY_LOG(info, "Portal {} is full! Destroying the portal!", portalEntity, entityInPortal);
                    registryWrapper.Destroy(portalEntity);
                    auto portalPosWorld = coordinatesTransformer.PhysicsToWorld(portalPos);
                    objectsFactory.SpawnPlayer(portalPosWorld, "Rescued player");
                    return;
                }

                registryWrapper.Destroy(entityInPortal);
            }
        });
}

void GameLogicSystem::ScatterPortalsIsTheyCloseToEachOther()
{
    auto portalEntities = registry.view<PhysicsComponent, PortalComponent>();
    for (auto entity : portalEntities)
    {
        auto& physicsComponent = portalEntities.get<PhysicsComponent>(entity);
        auto portalBody = physicsComponent.bodyRAII->GetBody();
        auto portalPos = portalBody->GetPosition();

        auto mergedPortals = request::FindEntitiesWithAllComponentsInRadius<PortalComponent>(registry, portalPos, 0.5f);
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

void GameLogicSystem::EatThePlayerByPortalIfCloser()
{
    auto portalEntities = registry.view<PhysicsComponent, PortalComponent>();
    portalEntities.each(
        [this](auto portalEntity, auto& physicsComponent, auto& portalComponent)
        {
            if (portalComponent.isSleeping)
                return;

            auto portalBody = physicsComponent.bodyRAII->GetBody();
            auto portalPos = portalBody->GetPosition();

            auto playerEntityOpt = request::FindClosestEntityWithAllComponents<PlayerComponent>(
                registry, portalPos,
                [this]([[maybe_unused]] auto entity)
                {
                    auto& physicsComponent = registry.get<PhysicsComponent>(entity);
                    auto body = physicsComponent.bodyRAII->GetBody();
                    return body->IsEnabled();
                });

            if (!playerEntityOpt.has_value())
                return;

            auto playerEntity = playerEntityOpt.value();

            auto& playerPhysicsComponent = registry.get<PhysicsComponent>(playerEntity);
            auto playerBody = playerPhysicsComponent.bodyRAII->GetBody();
            if (!playerBody->IsEnabled())
                return;
            auto playerBodyPos = playerBody->GetPosition();

            auto portalEatPlayerWithDistance = utils::GetConfig<float, "GameLogicSystem.portalEatPlayerWithDistance">();
            if (b2Distance(portalPos, playerBodyPos) < portalEatPlayerWithDistance)
            {
                MY_LOG(info, "Player {} is eaten by the portal {}!", playerEntity, portalEntity);
                registryWrapper.Destroy(playerEntity);
            }
        });
}
