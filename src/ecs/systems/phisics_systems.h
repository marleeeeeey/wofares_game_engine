#pragma once
#include <ecs/components/game_state_component.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <utils/coordinates_transformer.h>

class PhysicsSystem
{
    entt::registry& registry;
    GameState& gameState;
    std::shared_ptr<b2World> physicsWorld;
    CoordinatesTransformer coordinatesTransformer;
public:
    PhysicsSystem(entt::registry& registry);
    void Update(float deltaTime);
private:
    void RemoveDistantObjects();
    void UpdatePlayersWeaponDirection();
    void UpdateCollisionDisableTimerComponent(float deltaTime);
};
