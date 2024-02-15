#pragma once
#include "ecs/components/all_components.h"
#include "entt/entity/fwd.hpp"
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