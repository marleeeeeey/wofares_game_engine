#pragma once
#include "utils/game_options.h"
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <utils/box2d/box2d_body_tuner.h>
#include <utils/coordinates_transformer.h>
#include <utils/entt/entt_registry_wrapper.h>
#include <utils/factories/objects_factory.h>

class GameLogicSystem
{
    entt::registry& registry;
    EnttRegistryWrapper registryWrapper;
    Box2dBodyTuner bodyTuner;
    ObjectsFactory& objectsFactory;
    CoordinatesTransformer coordinatesTransformer;
    GameOptions& gameState;
public:
    GameLogicSystem(entt::registry& registry, ObjectsFactory& objectsFactory);
    void Update(float deltaTime);
private: // Portal logic
    void UpdatePortalsPosition(float deltaTime);
    std::optional<b2Vec2> FindPortalTargetPos(b2Vec2 portalPos);
    void MagnetDesctructibleParticlesToPortal(float deltaTime);
    void DestroyClosestDestructibleParticlesInPortal();
    void ScatterPortalsIsTheyCloseToEachOther();
    void EatThePlayerByPortalIfCloser();
    void CheckGameCompletness();
};