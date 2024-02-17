#pragma once
#include <ecs/components/game_state_component.h>
#include <entt/entt.hpp>
#include <utils/box2d_entt_contact_listener.h>

class WeaponControlSystem
{
    entt::registry& registry;
    GameState& gameState;
    Box2dEnttContactListener& contactListener;
    float deltaTime;
public:
    WeaponControlSystem(entt::registry& registry, Box2dEnttContactListener& contactListener);
    void Update(float deltaTime);
private:
    void ImpactTargetsInGrenadesExplosionRadius();
private: // Helper methods.
    std::vector<entt::entity> GetPhysicalBodiesNearGrenade(
        const b2Vec2& grenadePhysicsPos, float grenadeExplosionRadius);
    void ApplyForceToPhysicalBodies(std::vector<entt::entity> physicalEntities, const b2Vec2& grenadePhysicsPos);
};