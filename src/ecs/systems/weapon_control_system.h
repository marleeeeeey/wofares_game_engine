#pragma once
#include <ecs/components/game_state_component.h>
#include <entt/entt.hpp>
#include <queue>
#include <utils/box2d_entt_contact_listener.h>

class WeaponControlSystem
{
    entt::registry& registry;
    GameState& gameState;
    Box2dEnttContactListener& contactListener;
    float deltaTime;
    std::queue<entt::entity> explosionEntities;
public:
    WeaponControlSystem(entt::registry& registry, Box2dEnttContactListener& contactListener);
    void Update(float deltaTime);
private:
    void UpdateTimerExplosionComponents();
    void ProcessExplosionEntitiesQueue();
    void OnBazookaContactWithTile(entt::entity bazookaEntity, entt::entity tileEntity);
    void TryToRunExplosionImpactComponent(entt::entity explosionEntity);
private: // Low level functions.
    std::vector<entt::entity> GetPhysicalBodiesNearGrenade(
        const b2Vec2& grenadePhysicsPos, float grenadeExplosionRadius);
    void ApplyForceToPhysicalBodies(
        std::vector<entt::entity> physicalEntities, const b2Vec2& grenadePhysicsPos, float force);
    // Disable collisions after some time.
    void StartCollisionDisableTimer(const std::vector<entt::entity>& physicalEntities);
};