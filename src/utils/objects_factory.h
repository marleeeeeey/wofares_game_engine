#pragma once
#include "utils/resource_manager.h"
#include <entt/entt.hpp>
#include <utils/box2d_body_creator.h>
#include <utils/game_options.h>


class ObjectsFactory
{
    entt::registry& registry;
    ResourceManager& resourceManager;
    GameOptions& gameState;
    std::shared_ptr<b2World> physicsWorld;
    Box2dBodyCreator box2dBodyCreator;
public:
    ObjectsFactory(entt::registry& registry, ResourceManager& resourceManager);
    entt::entity createPlayer(const glm::vec2& playerSdlWorldPos);
};