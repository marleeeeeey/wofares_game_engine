#include "camera_control_system.h"
#include <SDL_mouse.h>
#include <ecs/components/game_components.h>
#include <utils/game_options.h>

CameraControlSystem::CameraControlSystem(entt::registry& registry, InputEventManager& inputEventManager)
  : registry(registry), gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())),
    inputEventManager(inputEventManager), transformer(registry)
{
    inputEventManager.Subscribe([this](const InputEventManager::EventInfo& eventInfo)
                                { HandleCameraMovementAndScale(eventInfo.originalEvent); });
    inputEventManager.Subscribe([this](const InputEventManager::EventInfo& eventInfo)
                                { HandleMouseScreenPosition(eventInfo.originalEvent); });
}

void CameraControlSystem::HandleCameraMovementAndScale(const SDL_Event& event)
{
    auto& gameState = registry.get<GameOptions>(registry.view<GameOptions>().front());
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
            gameState.windowOptions.cameraCenterSdl;

        // Calculate the new position of the camera so that the point under the cursor remains in the same place
        gameState.windowOptions.cameraCenterSdl = mouseWorldBeforeZoom -
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
        gameState.windowOptions.cameraCenterSdl.x -= deltaX;
        gameState.windowOptions.cameraCenterSdl.y -= deltaY;
    }
};

void CameraControlSystem::HandleMouseScreenPosition(const SDL_Event& event)
{
    auto& gameState = registry.get<GameOptions>(registry.view<GameOptions>().front());
    if (event.type == SDL_MOUSEMOTION)
    {
        gameState.windowOptions.lastMousePosInWindow = glm::vec2(event.motion.x, event.motion.y);
    }
};

void CameraControlSystem::Update(float deltaTime)
{
    if (!gameState.controlOptions.isSceneCaptured)
        PosiotioningCameraToPlayer(deltaTime);
};

void CameraControlSystem::PosiotioningCameraToPlayer(float deltaTime)
{
    auto players = registry.view<PlayerInfo, PhysicsInfo>();
    for (auto entity : players)
    {
        const auto& [playerInfo, physicalBody] = players.get<PlayerInfo, PhysicsInfo>(entity);
        auto playerBody = physicalBody.bodyRAII->GetBody();
        auto playerPosSdl = transformer.PhysicsToWorld(playerBody->GetPosition());
        auto& cameraCenterSdl = gameState.windowOptions.cameraCenterSdl;

        // Smoothing factor (value between 0 and 1, where closer to 0 - smoother following)
        float smoothFactor = 0.1f;

        glm::vec2 diffSdl = playerPosSdl - cameraCenterSdl;
        float distanceSdl = glm::length(diffSdl);

        if (distanceSdl > 0.001f)
        {
            // "Ease out" interpolation
            float factor = 1.0f - pow(1.0f - smoothFactor, deltaTime * 60.0f);
            cameraCenterSdl += diffSdl * factor;
        }
    }
};
