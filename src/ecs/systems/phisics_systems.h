#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <utils/coordinates_transformer.h>
#include <utils/game_options.h>


class PhysicsSystem
{
    entt::registry& registry;
    GameOptions& gameState;
    std::shared_ptr<b2World> physicsWorld;
    CoordinatesTransformer coordinatesTransformer;
public:
    PhysicsSystem(entt::registry& registry);
    void Update(float deltaTime);
private:
    void RemoveDistantObjects();
    void UpdatePlayersWeaponDirection();
    void UpdateCollisionDisableTimerComponent(float deltaTime);
    void SetPlayersRotationToZero();
};
