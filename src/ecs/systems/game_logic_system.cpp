#include "game_logic_system.h"
#include <ecs/components/physics_components.h>
#include <ecs/components/player_components.h>
#include <ecs/components/portal_components.h>
#include <utils/entt/entt_registry_requests.h>
#include <utils/glm_box2d_conversions.h>

GameLogicSystem::GameLogicSystem(entt::registry& registry) : registry(registry), registryWrapper(registry)
{}

void GameLogicSystem::Update(float deltaTime)
{
    UpdatePortalObjectsPosition(deltaTime);
    MagnetDesctructibleParticlesToPortal(deltaTime);
    DestroyClosestDestructibleParticlesInPortal();
}

void GameLogicSystem::UpdatePortalObjectsPosition(float deltaTime)
{
    auto portalEntities = registry.view<PhysicsComponent, PortalComponent>();
    for (auto entity : portalEntities)
    {
        auto& portalComponent = portalEntities.get<PortalComponent>(entity);
        auto& physicsComponent = portalEntities.get<PhysicsComponent>(entity);
        auto portalBody = physicsComponent.bodyRAII->GetBody();
        auto portalPos = portalBody->GetPosition();

        auto closestTargetPosOpt = FindPortalTargetPos(portalPos);
        if (!closestTargetPosOpt.has_value())
            continue;

        // Update the position of the portal object.
        b2Vec2 direction = closestTargetPosOpt.value() - portalPos;
        direction.Normalize();
        b2Vec2 newPos = portalPos + direction * portalComponent.speed * deltaTime;
        portalBody->SetTransform(newPos, portalBody->GetAngle());
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
            // use delta time to make the speed independent of the frame rate.
            // portalComponent.magnetForce is the force of the magnet.
            bodyToMagnet->ApplyForceToCenter(portalComponent.magnetForce * direction * deltaTime, true);
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
