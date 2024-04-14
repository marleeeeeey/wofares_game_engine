#include "game_logic_system.h"
#include <ecs/components/physics_components.h>
#include <ecs/components/player_components.h>
#include <ecs/components/portal_components.h>
#include <utils/entt/entt_registry_requests.h>
#include <utils/glm_box2d_conversions.h>

GameLogicSystem::GameLogicSystem(entt::registry& registry) : registry(registry)
{}

void GameLogicSystem::Update(float deltaTime)
{
    UpdatePortalObjectsPosition(deltaTime);
}

void GameLogicSystem::UpdatePortalObjectsPosition(float deltaTime)
{
    auto portalEntities = registry.view<PhysicsComponent, PortalComponent>();
    for (auto entity : portalEntities)
    {
        auto& portalComponent = portalEntities.get<PortalComponent>(entity);
        auto& physicsComponent = portalEntities.get<PhysicsComponent>(entity);
        auto portalBody = physicsComponent.bodyRAII->GetBody();

        // Find the closest target position.
        b2Vec2 closestTargetPos;
        auto closestStickyPos = FindClosestTargetPos<StickyComponent>(registry, portalBody->GetPosition());
        if (closestStickyPos.has_value())
        {
            closestTargetPos = closestStickyPos.value();
        }
        else
        {
            auto closestPlayerPos = FindClosestTargetPos<PlayerComponent>(registry, portalBody->GetPosition());
            if (!closestPlayerPos.has_value())
            {
                return;
            }
            closestTargetPos = closestPlayerPos.value();
        }

        // Update the position of the portal object.
        // Move it closer to the player.
        b2Vec2 direction = closestTargetPos - portalBody->GetPosition();
        direction.Normalize();
        b2Vec2 newPos = portalBody->GetPosition() + direction * portalComponent.speed * deltaTime;
        portalBody->SetTransform(newPos, portalBody->GetAngle());
    }
}
