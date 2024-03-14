#pragma once
#include "utils/factories/box2d_body_creator.h"
#include "utils/game_options.h"
#include <box2d/box2d.h>
#include <entt/entt.hpp>
#include <vector>

class PhysicsMethods
{
    entt::registry& registry;
    GameOptions& gameState;
    Box2dBodyCreator box2dBodyCreator;
    CoordinatesTransformer coordinatesTransformer;
public:
    PhysicsMethods(entt::registry& registry);
public: // Client methods.
    void ApplyForceToPhysicalBodies(
        std::vector<entt::entity> physicalEntities, const glm::vec2& forceCenterWorld, float force);
};