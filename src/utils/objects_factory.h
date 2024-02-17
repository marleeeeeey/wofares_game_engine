#pragma once
#include <ecs/components/game_state_component.h>
#include <entt/entt.hpp>

class ObjectsFactory
{
    entt::registry& registry;
    GameState& gameState;
    std::shared_ptr<b2World> physicsWorld;
public:
    ObjectsFactory(entt::registry& registry);
    entt::entity createPlayer(const glm::vec2& playerSdlWorldPos);
};