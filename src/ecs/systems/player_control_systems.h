#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <utils/box2d_body_creator.h>
#include <utils/box2d_entt_contact_listener.h>
#include <utils/coordinates_transformer.h>
#include <utils/entt_registry_wrapper.h>
#include <utils/input_event_manager.h>

class PlayerControlSystem
{
    EnttRegistryWrapper& registryWrapper;
    entt::registry& registry;
    InputEventManager& inputEventManager;
    GameOptions& gameState;
    CoordinatesTransformer transformer;
    Box2dBodyCreator box2dBodyCreator;
    Box2dEnttContactListener& contactListener;
public:
    PlayerControlSystem(
        EnttRegistryWrapper& registryWrapper, InputEventManager& inputEventManager,
        Box2dEnttContactListener& contactListener);
private: // Callbacks for the InputEventManager.
    void HandlePlayerMovement(const InputEventManager::EventInfo& eventInfo);
    void HandlePlayerAttack(const InputEventManager::EventInfo& eventInfo);
    void HandlePlayerBuildingAction(const InputEventManager::EventInfo& eventInfo);
    void HandlePlayerWeaponDirection(const InputEventManager::EventInfo& eventInfo);
private: // Methods to set the ground contact flag.
    void HandlePlayerBeginSensorContact(entt::entity entityA, entt::entity entityB);
    void HandlePlayerEndSensorContact(entt::entity entityA, entt::entity entityB);
    void SetGroundContactFlagIfPlayer(entt::entity entity, bool value);
private: // Low level functions.
    entt::entity SpawnFlyingEntity(
        const glm::vec2& sdlPos, const glm::vec2& sdlSize, const glm::vec2& forceDirection, float force);
};