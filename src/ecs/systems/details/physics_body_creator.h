#pragma once
#include <utils/box2d_RAII.h>

// Convert from SDL to Box2D system.
std::shared_ptr<Box2dObjectRAII> CreateStaticPhysicsBody(
    std::shared_ptr<b2World> physicsWorld, const glm::vec2& sdlPos, const glm::vec2& sdlSize);

// Convert from SDL to Box2D system.
std::shared_ptr<Box2dObjectRAII> CreateDynamicPhysicsBody(
    std::shared_ptr<b2World> physicsWorld, const glm::vec2& sdlPos, const glm::vec2& sdlSize);
