#pragma once
#include <ecs/components/game_state_component.h>
#include <entt/entt.hpp>
#include <utils/box2d_body_creator.h>


class ObjectsFactory
{
    entt::registry& registry;
    GameState& gameState;
    std::shared_ptr<b2World> physicsWorld;
    Box2dBodyCreator box2dBodyCreator;
public:
    ObjectsFactory(entt::registry& registry);
    entt::entity createPlayer(const glm::vec2& playerSdlWorldPos);
};