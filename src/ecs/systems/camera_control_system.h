#pragma once
#include <entt/entt.hpp>
#include <utils/input_event_manager.h>

class CameraControlSystem
{
    entt::registry& registry;
    InputEventManager& inputEventManager;
public:
    CameraControlSystem(entt::registry& registry, InputEventManager& inputEventManager);
private:
    void HandleCameraMovementAndScale(const SDL_Event& event);
    void HandleMouseScreenPosition(const SDL_Event& event);
};