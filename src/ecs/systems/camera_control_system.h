#pragma once
#include "utils/coordinates_transformer.h"
#include <ecs/components/game_state_component.h>
#include <entt/entt.hpp>
#include <utils/input_event_manager.h>

class CameraControlSystem
{
    entt::registry& registry;
    GameState& gameState;
    InputEventManager& inputEventManager;
    CoordinatesTransformer transformer;
public:
    CameraControlSystem(entt::registry& registry, InputEventManager& inputEventManager);
    void Update(float deltaTime);
private:
    void HandleCameraMovementAndScale(const SDL_Event& event);
    void HandleMouseScreenPosition(const SDL_Event& event);
    void PosiotioningCameraToPlayer(float deltaTime);
};