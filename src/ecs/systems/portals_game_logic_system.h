#pragma once
#include "utils/game_options.h"
#include "utils/systems/audio_system.h"
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <utils/box2d/box2d_body_tuner.h>
#include <utils/coordinates_transformer.h>
#include <utils/entt/entt_registry_wrapper.h>
#include <utils/factories/objects_factory.h>

// TODO1: Remove this class later because of implementing new game.
class PortalsGameLogicSystem
{
    entt::registry& registry;
    EnttRegistryWrapper registryWrapper;
    Box2dBodyTuner bodyTuner;
    ObjectsFactory& objectsFactory;
    CoordinatesTransformer coordinatesTransformer;
    GameOptions& gameState;
    AudioSystem& audioSystem;
public:
    PortalsGameLogicSystem(entt::registry& registry, ObjectsFactory& objectsFactory, AudioSystem& audioSystem);
    void Update(float deltaTime);
private: ///////////////////////////////////////// Portal logic. ///////////////////////////////////////
    void UpdatePortalsPosition(float deltaTime);
    void UpdatePortalTarget(entt::entity portalEntity);
    void MagnetFoodToPortal(float deltaTime);
    void DestroyClosestFoodInPortal();
    void ScatterPortalsIsTheyCloseToEachOther();
    void EatThePlayerByPortalIfCloser();
    void CheckGameCompletness();
};