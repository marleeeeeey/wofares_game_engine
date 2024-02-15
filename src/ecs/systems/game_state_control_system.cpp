#include "game_state_control_system.h"
#include <ecs/components/all_components.h>

void SubscribeGameStateControlSystem(entt::registry& registry, InputEventManager& inputEventManager)
{
    inputEventManager.subscribeRawListener(
        [&registry](const SDL_Event& event)
        {
            auto& gameState = registry.get<GameState>(registry.view<GameState>().front());
            bool isEscPressed = event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE;

            if (isEscPressed || event.type == SDL_QUIT)
            {
                gameState.controlOptions.quit = true;
            }
        });

    // subcrive on continuous events on space key and copy duration of pressing to the game state
    inputEventManager.subscribeСontinuousListener(
        InputEventManager::EventType::ButtonHold,
        [&registry](const InputEventManager::EventInfo& eventInfo)
        {
            auto& gameState = registry.get<GameState>(registry.view<GameState>().front());
            auto& originalEvent = eventInfo.originalEvent;

            if (originalEvent.key.keysym.scancode == SDL_SCANCODE_SPACE)
            {
                gameState.debugInfo.spacePressedDuration = eventInfo.holdDuration;
            }
        });

    inputEventManager.subscribeСontinuousListener(
        InputEventManager::EventType::ButtonReleaseAfterHold,
        [&registry](const InputEventManager::EventInfo& eventInfo)
        {
            auto& gameState = registry.get<GameState>(registry.view<GameState>().front());
            auto& originalEvent = eventInfo.originalEvent;

            if (originalEvent.key.keysym.scancode == SDL_SCANCODE_SPACE)
            {
                gameState.debugInfo.spacePressedDurationOnUpEvent = eventInfo.holdDuration;
            }
        });
}
