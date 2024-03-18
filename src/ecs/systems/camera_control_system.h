#pragma once
#include "utils/coordinates_transformer.h"
#include <entt/entt.hpp>
#include <utils/game_options.h>
#include <utils/systems/input_event_manager.h>

class CameraControlSystem
{
    entt::registry& registry;
    GameOptions& gameState;
    InputEventManager& inputEventManager;
    CoordinatesTransformer coordinatesTransformer;
public:
    CameraControlSystem(entt::registry& registry, InputEventManager& inputEventManager);
    void Update(float deltaTime);
private:
    void HandleCameraMovementAndScale(const SDL_Event& event);
    void HandleMouseScreenPosition(const SDL_Event& event);
    void PositioningCameraToPlayer(float deltaTime);
};