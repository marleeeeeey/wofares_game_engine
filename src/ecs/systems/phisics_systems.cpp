#include "phisics_systems.h"
#include <ecs/components/physics_components.h>
#include <ecs/components/player_components.h>
#include <glm/glm.hpp>
#include <my_common_cpp_utils/config.h>
#include <utils/entt_registry_wrapper.h>
#include <utils/glm_box2d_conversions.h>
#include <utils/math_utils.h>

PhysicsSystem::PhysicsSystem(EnttRegistryWrapper& registryWrapper)
  : registryWrapper(registryWrapper), registry(registryWrapper.GetRegistry()),
    gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())), physicsWorld(gameState.physicsWorld),
    coordinatesTransformer(registry)
{}

void PhysicsSystem::Update(float deltaTime)
{
    // Update the physics world with Box2D engine.
    auto& velocityIterations = utils::GetConfig<int, "PhysicsSystem.velocityIterations">();
    auto& positionIterations = utils::GetConfig<int, "PhysicsSystem.positionIterations">();
    physicsWorld->Step(deltaTime, velocityIterations, positionIterations);

    UpdateAngleRegardingWithAnglePolicy();
    UpdatePlayersWeaponDirection();
    RemoveDistantObjects();
};

void PhysicsSystem::RemoveDistantObjects()
{
    auto levelBounds = gameState.levelOptions.levelBox2dBounds;

    auto physicalBodies = registry.view<PhysicsComponent>();
    for (auto entity : physicalBodies)
    {
        auto& physicalBody = physicalBodies.get<PhysicsComponent>(entity);
        b2Vec2 posPhysics = physicalBody.bodyRAII->GetBody()->GetPosition();

        if (!utils::IsPointInsideBounds(posPhysics, levelBounds))
        {
            registryWrapper.Destroy(entity);
        }
    }
}

// Set the direction of the weapon of the player to the last mouse position.
void PhysicsSystem::UpdatePlayersWeaponDirection()
{
    auto players = registry.view<PhysicsComponent, PlayerComponent>();
    for (auto entity : players)
    {
        const auto& [physicalBody, playerInfo] = players.get<PhysicsComponent, PlayerComponent>(entity);

        auto& lastMousePosInWindow = gameState.windowOptions.lastMousePosInWindow;
        glm::vec2 playerPosInWindow =
            coordinatesTransformer.PhysicsToScreen(physicalBody.bodyRAII->GetBody()->GetPosition());

        playerInfo.weaponDirection = glm::normalize(lastMousePosInWindow - playerPosInWindow);
    }
}

void PhysicsSystem::UpdateAngleRegardingWithAnglePolicy()
{
    auto physicsComponens = registry.view<PhysicsComponent>();
    for (auto entity : physicsComponens)
    {
        auto& physicsComponent = physicsComponens.get<PhysicsComponent>(entity);
        auto body = physicsComponent.bodyRAII->GetBody();

        if (physicsComponent.anglePolicy == AnglePolicy::VelocityDirection)
        {
            b2Vec2 velocity = body->GetLinearVelocity();
            float angle = atan2(velocity.y, velocity.x);
            body->SetTransform(body->GetPosition(), angle);
        }

        if (physicsComponent.anglePolicy == AnglePolicy::Fixed)
        {
            body->SetFixedRotation(true);
        }

        if (physicsComponent.anglePolicy == AnglePolicy::Dynamic)
        {
            body->SetFixedRotation(false);
        }
    }
};