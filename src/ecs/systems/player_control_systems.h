#pragma once
#include "entt/entity/fwd.hpp"
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <utils/coordinates_transformer.h>
#include <utils/input_event_manager.h>

class PlayerControlSystem
{
    entt::registry& registry;
    InputEventManager& inputEventManager;
    GameState& gameState;
    CoordinatesTransformer transformer; // TODO: think to put it in the GameState class.
public:
    PlayerControlSystem(entt::registry& registry, InputEventManager& inputEventManager);
private: // Callbacks for the InputEventManager.
    void HandlePlayerMovement(const InputEventManager::EventInfo& eventInfo);
    void HandlePlayerAttack(const InputEventManager::EventInfo& eventInfo);
    void HandlePlayerBuildingAction(const SDL_Event& event); // TODO: remplace with eventInfo
    void HandlePlayerWeaponDirection(const SDL_Event& event); // TODO: remplace with eventInfo
private: // Low level functions.
    entt::entity SpawnFlyingEntity(
        const glm::vec2& sdlPos, const glm::vec2& sdlSize, const glm::vec2& forceDirection, float force);
};