#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <utils/coordinates_transformer.h>
#include <utils/entt_registry_wrapper.h>
#include <utils/game_options.h>

class PhysicsSystem
{
    EnttRegistryWrapper& registryWrapper;
    entt::registry& registry;
    GameOptions& gameState;
    std::shared_ptr<b2World> physicsWorld;
    CoordinatesTransformer coordinatesTransformer;
public:
    PhysicsSystem(EnttRegistryWrapper& registryWrapper);
    void Update(float deltaTime);
private:
    void RemoveDistantObjects();
    void UpdatePlayersWeaponDirection();
    void UpdateAngleRegardingWithAnglePolicy();
    void UpdatePortalObjectsPosition(float deltaTime);
};
