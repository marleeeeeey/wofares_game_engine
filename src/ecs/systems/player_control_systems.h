#pragma once
#include "utils/factories/objects_factory.h"
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <utils/coordinates_transformer.h>
#include <utils/entt_registry_wrapper.h>
#include <utils/factories/box2d_body_creator.h>
#include <utils/systems/box2d_entt_contact_listener.h>
#include <utils/systems/input_event_manager.h>

class PlayerControlSystem
{
    EnttRegistryWrapper& registryWrapper;
    entt::registry& registry;
    InputEventManager& inputEventManager;
    GameOptions& gameState;
    CoordinatesTransformer transformer;
    Box2dBodyCreator box2dBodyCreator;
    Box2dEnttContactListener& contactListener;
    ObjectsFactory& objectsFactory;
public:
    PlayerControlSystem(
        EnttRegistryWrapper& registryWrapper, InputEventManager& inputEventManager,
        Box2dEnttContactListener& contactListener, ObjectsFactory& objectsFactory);
private: // Callbacks for the InputEventManager.
    void HandlePlayerMovement(const InputEventManager::EventInfo& eventInfo);
    void HandlePlayerAttack(const InputEventManager::EventInfo& eventInfo);
    void HandlePlayerBuildingAction(const InputEventManager::EventInfo& eventInfo);
    void HandlePlayerWeaponDirection(const InputEventManager::EventInfo& eventInfo);
private: // Methods to set the ground contact flag.
    void HandlePlayerBeginSensorContact(entt::entity entityA, entt::entity entityB);
    void HandlePlayerEndSensorContact(entt::entity entityA, entt::entity entityB);
    void SetGroundContactFlagIfPlayer(entt::entity entity, bool value);
};