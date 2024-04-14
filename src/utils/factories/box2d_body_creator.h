#pragma once
#include "entt/entity/fwd.hpp"
#include "utils/box2d/box2d_body_tuner.h"
#include <ecs/components/physics_components.h>
#include <utils/RAII/box2d_RAII.h>
#include <utils/box2d/box2d_body_options.h>
#include <utils/coordinates_transformer.h>
#include <utils/game_options.h>

class Box2dBodyCreator
{
    const std::shared_ptr<b2World> physicsWorld;
    Box2dBodyTuner bodyTuner;
public:
    Box2dBodyCreator(entt::registry& registry);
    // entity is used to store entity in the physics body.
    PhysicsComponent& CreatePhysicsBody(
        entt::entity entity, const glm::vec2& posWorld, const Box2dBodyOptions& options);
    PhysicsComponent& CreatePhysicsBody(
        entt::entity entity, const glm::vec2& posWorld, const glm::vec2& sizeWorld, Box2dBodyOptions options = {});
};