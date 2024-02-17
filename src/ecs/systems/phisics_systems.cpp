#include "phisics_systems.h"
#include <ecs/components/game_components.h>
#include <glm/glm.hpp>
#include <utils/glm_box2d_conversions.h>

PhysicsSystem::PhysicsSystem(entt::registry& registry)
  : registry(registry), gameState(registry.get<GameState>(registry.view<GameState>().front())),
    physicsWorld(gameState.physicsWorld), coordinatesTransformer(registry)
{}

void PhysicsSystem::Update(float deltaTime)
{
    // Update the physics world with Box2D engine.
    physicsWorld->Step(
        deltaTime, gameState.physicsOptions.velocityIterations, gameState.physicsOptions.positionIterations);

    UpdatePlayersWeaponDirection();
    RemoveDistantObjectsSystem();
};

void PhysicsSystem::RemoveDistantObjectsSystem()
{
    auto levelBounds = gameState.levelOptions.levelBox2dBounds;

    auto physicalBodies = registry.view<PhysicalBody>();
    for (auto entity : physicalBodies)
    {
        auto& physicalBody = physicalBodies.get<PhysicalBody>(entity);
        b2Vec2 pos = physicalBody.value->GetBody()->GetPosition();

        if (!IsPointInsideBounds(pos, levelBounds))
        {
            registry.destroy(entity);
        }
    }
}

// Set the direction of the weapon of the player to the last mouse position.
void PhysicsSystem::UpdatePlayersWeaponDirection()
{
    auto players = registry.view<PlayerNumber, PhysicalBody, PlayersWeaponDirection>();
    for (auto entity : players)
    {
        const auto& [physicalBody, weaponDirection] = players.get<PhysicalBody, PlayersWeaponDirection>(entity);

        auto& lastMousePosInWindow = gameState.windowOptions.lastMousePosInWindow;
        glm::vec2 playerPosInWindow =
            coordinatesTransformer.PhysicsToCamera(physicalBody.value->GetBody()->GetPosition());

        weaponDirection.value = glm::normalize(lastMousePosInWindow - playerPosInWindow);
    }
}