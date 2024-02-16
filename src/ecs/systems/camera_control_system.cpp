#include "camera_control_system.h"
#include <SDL_mouse.h>
#include <ecs/components/game_components.h>
#include <ecs/components/game_state_component.h>

void SubscribeCameraControlSystem(entt::registry& registry, InputEventManager& inputEventManager)
{
    inputEventManager.SubscribeRawListener(
        [&registry](const SDL_Event& event)
        {
            auto& gameState = registry.get<GameState>(registry.view<GameState>().front());
            if (event.type == SDL_MOUSEWHEEL)
            {
                float prevScale = gameState.windowOptions.cameraScale;

                // Calculate the new scale of the camera
                const float scaleSpeed = 2.0f;
                if (event.wheel.y > 0)
                    gameState.windowOptions.cameraScale *= scaleSpeed;
                else if (event.wheel.y < 0)
                    gameState.windowOptions.cameraScale /= scaleSpeed;
                gameState.windowOptions.cameraScale = glm::clamp(gameState.windowOptions.cameraScale, 0.25f, 8.0f);

                // Get the cursor coordinates in world coordinates
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                glm::vec2 mouseWorldBeforeZoom =
                    (glm::vec2(mouseX, mouseY) - gameState.windowOptions.windowSize * 0.5f) / prevScale +
                    gameState.windowOptions.cameraCenter;

                // Calculate the new position of the camera so that the point under the cursor remains in the same place
                gameState.windowOptions.cameraCenter = mouseWorldBeforeZoom -
                    (glm::vec2(mouseX, mouseY) - gameState.windowOptions.windowSize * 0.5f) /
                        gameState.windowOptions.cameraScale;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_MIDDLE)
            {
                gameState.controlOptions.isSceneCaptured = true;
            }
            else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_MIDDLE)
            {
                gameState.controlOptions.isSceneCaptured = false;
            }
            else if (event.type == SDL_MOUSEMOTION && gameState.controlOptions.isSceneCaptured)
            {
                float deltaX = event.motion.xrel / gameState.windowOptions.cameraScale;
                float deltaY = event.motion.yrel / gameState.windowOptions.cameraScale;
                gameState.windowOptions.cameraCenter.x -= deltaX;
                gameState.windowOptions.cameraCenter.y -= deltaY;
            }
        });

    // subscribe listener to any mouse movement and save coordinates of the mouse to the gameState as the last mouse
    // position.
    inputEventManager.SubscribeRawListener(
        [&registry](const SDL_Event& event)
        {
            auto& gameState = registry.get<GameState>(registry.view<GameState>().front());
            if (event.type == SDL_MOUSEMOTION)
            {
                gameState.windowOptions.lastMousePosInWindow = glm::vec2(event.motion.x, event.motion.y);
            }
        });
}
