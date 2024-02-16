#pragma once
#include <ecs/components/game_state_component.h>
#include <entt/entt.hpp>

class WeaponControlSystem
{
    entt::registry& registry;
    GameState& gameState;
    float deltaTime;
public:
    WeaponControlSystem(entt::registry& registry, float deltaTime);
    // TODO: add here method to subscribe to input events.
private:
    void GrenadeExpiryControl();
};