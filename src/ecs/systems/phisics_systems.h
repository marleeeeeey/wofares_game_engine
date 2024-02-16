#pragma once
#include "ecs/systems/details/coordinates_transformer.h"
#include <ecs/components/all_components.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>

class PhysicsSystem
{
    entt::registry& registry;
    GameState& gameState;
    std::shared_ptr<b2World> physicsWorld;
    CoordinatesTransformer coordinatesTransformer;
    float deltaTime;
public:
    PhysicsSystem(entt::registry& registry, float deltaTime);
private:
    void RemoveDistantObjectsSystem();
    void UpdatePlayersWeaponDirection();
};
