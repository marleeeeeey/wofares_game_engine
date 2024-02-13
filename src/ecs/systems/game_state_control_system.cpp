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
}
