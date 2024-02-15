#pragma once
#include "ecs/systems/details/coordinates_transformer.h"
#include <utils/box2d_RAII.h>

// Convert from SDL to Box2D system.
std::shared_ptr<Box2dObjectRAII> CreateStaticPhysicsBody(
    const CoordinatesTransformer& coordinatesTransformer, std::shared_ptr<b2World> physicsWorld,
    const glm::vec2& sdlPos, const glm::vec2& sdlSize);

// Convert from SDL to Box2D system.
std::shared_ptr<Box2dObjectRAII> CreateDynamicPhysicsBody(
    const CoordinatesTransformer& coordinatesTransformer, std::shared_ptr<b2World> physicsWorld,
    const glm::vec2& sdlPos, const glm::vec2& sdlSize);
