#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <utils/coordinates_transformer.h>

class GameLogicSystem
{
    entt::registry& registry;
public:
    GameLogicSystem(entt::registry& registry);
    void Update(float deltaTime);
private:
    void UpdatePortalObjectsPosition(float deltaTime);
};