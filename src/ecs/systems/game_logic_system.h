#pragma once
#include "utils/box2d/box2d_body_tuner.h"
#include "utils/entt/entt_registry_wrapper.h"
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <utils/coordinates_transformer.h>

class GameLogicSystem
{
    entt::registry& registry;
    EnttRegistryWrapper registryWrapper;
    Box2dBodyTuner bodyTuner;
public:
    GameLogicSystem(entt::registry& registry);
    void Update(float deltaTime);
private: // Portal logic
    void UpdatePortalsPosition(float deltaTime);
    std::optional<b2Vec2> FindPortalTargetPos(b2Vec2 portalPos);
    void MagnetDesctructibleParticlesToPortal(float deltaTime);
    void DestroyClosestDestructibleParticlesInPortal();
    void ScatterPortalsIsTheyCloseToEachOther();
    void EatThePlayerByPortalIfCloser();
};