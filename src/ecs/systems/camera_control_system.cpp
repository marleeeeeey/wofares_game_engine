#include "camera_control_system.h"
#include <ecs/components/all_components.h>

void SubscribeCameraControlSystem(entt::registry& registry, InputEventManager& inputEventManager)
{
    inputEventManager.subscribeRawListener(
        [&registry](const SDL_Event& event)
        {
            auto& gameState = registry.get<GameState>(registry.view<GameState>().front());
            if (event.type == SDL_MOUSEWHEEL)
            {
                float prevScale = gameState.renderingOptions.cameraScale;

                // Calculate the new scale of the camera
                const float scaleSpeed = 1.3f;
                if (event.wheel.y > 0)
                    gameState.renderingOptions.cameraScale *= scaleSpeed;
                else if (event.wheel.y < 0)
                    gameState.renderingOptions.cameraScale /= scaleSpeed;
                gameState.renderingOptions.cameraScale = glm::clamp(gameState.renderingOptions.cameraScale, 0.2f, 6.0f);

                // Get the cursor coordinates in world coordinates
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                glm::vec2 mouseWorldBeforeZoom =
                    (glm::vec2(mouseX, mouseY) - gameState.renderingOptions.windowSize * 0.5f) / prevScale +
                    gameState.renderingOptions.cameraCenter;

                // Calculate the new position of the camera so that the point under the cursor remains in the same place
                gameState.renderingOptions.cameraCenter = mouseWorldBeforeZoom -
                    (glm::vec2(mouseX, mouseY) - gameState.renderingOptions.windowSize * 0.5f) /
                        gameState.renderingOptions.cameraScale;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT)
            {
                gameState.controlOptions.isSceneCaptured = true;
            }
            else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_RIGHT)
            {
                gameState.controlOptions.isSceneCaptured = false;
            }
            else if (event.type == SDL_MOUSEMOTION && gameState.controlOptions.isSceneCaptured)
            {
                float deltaX = event.motion.xrel / gameState.renderingOptions.cameraScale;
                float deltaY = event.motion.yrel / gameState.renderingOptions.cameraScale;
                gameState.renderingOptions.cameraCenter.x -= deltaX;
                gameState.renderingOptions.cameraCenter.y -= deltaY;
            }
        });
}
