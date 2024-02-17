#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <utils/input_event_manager.h>

class PlayerControlSystem
{
    entt::registry& registry;
    InputEventManager& inputEventManager;
public:
    PlayerControlSystem(entt::registry& registry, InputEventManager& inputEventManager);
private:
    void HandlePlayerMovement(const InputEventManager::EventInfo& eventInfo);
    void HandlePlayerAttack(const InputEventManager::EventInfo& eventInfo);
    void HandlePlayerBuildingAction(const SDL_Event& event); // TODO: remplace with eventInfo
    void HandlePlayerWeaponDirection(const SDL_Event& event); // TODO: remplace with eventInfo
};