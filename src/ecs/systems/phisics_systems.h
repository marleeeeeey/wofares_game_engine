#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <utils/coordinates_transformer.h>
#include <utils/entt/entt_registry_wrapper.h>
#include <utils/game_options.h>

class PhysicsSystem
{
    EnttRegistryWrapper& registryWrapper;
    entt::registry& registry;
    GameOptions& gameState;
    CoordinatesTransformer coordinatesTransformer;
public:
    PhysicsSystem(EnttRegistryWrapper& registryWrapper);
    void Update(float deltaTime);
private:
    void RemoveDistantObjects();
    void UpdatePlayersWeaponDirection();
    void UpdateAngleRegardingWithAnglePolicy();
};
