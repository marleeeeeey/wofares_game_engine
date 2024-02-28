#pragma once
#include <entt/entt.hpp>
#include <utils/input_event_manager.h>

class GameStateControlSystem
{
    entt::registry& registry;
    InputEventManager& inputEventManager;
public:
    GameStateControlSystem(entt::registry& registry, InputEventManager& inputEventManager);
private:
    void HandleGameStateChange(const SDL_Event& event);
private:
    void SubscribeToInputEvents();
private: // Debug purposes. Maybe removed safely.
    void HandleSpaceHoldButtonToDebugInfo(const InputEventManager::EventInfo& eventInfo);
    void HandleSpaceReleaseAfterHoldButtonToDebugInfo(const InputEventManager::EventInfo& eventInfo);
};