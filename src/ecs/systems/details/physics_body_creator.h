#pragma once
#include "entt/entity/fwd.hpp"
#include <ecs/components/game_state_component.h>
#include <ecs/systems/details/coordinates_transformer.h>
#include <utils/box2d_RAII.h>

// Convert from SDL to Box2D system.
// entity is used to store entity in the physics body.
std::shared_ptr<Box2dObjectRAII> CreateStaticPhysicsBody(
    entt::entity entity, const CoordinatesTransformer& coordinatesTransformer, std::shared_ptr<b2World> physicsWorld,
    const glm::vec2& sdlPos, const glm::vec2& sdlSize);

// Convert from SDL to Box2D system.
// entity is used to store entity in the physics body.
std::shared_ptr<Box2dObjectRAII> CreateDynamicPhysicsBody(
    entt::entity entity, const CoordinatesTransformer& coordinatesTransformer, std::shared_ptr<b2World> physicsWorld,
    const glm::vec2& sdlPos, const glm::vec2& sdlSize);
