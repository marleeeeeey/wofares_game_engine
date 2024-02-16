#pragma once
#include "box2d/b2_math.h"
#include <box2d/box2d.h>
#include <ecs/components/all_components.h>
#include <entt/entt.hpp>

// There are 3 coordinate systems in the game:
// 1. World (game) coordinates.
// 2. Camera (screen) coordinates.
// 3. Physics coordinates.
class CoordinatesTransformer
{
    GameState& gameState;
public:
    CoordinatesTransformer(entt::registry& registry);
    // World to Camera (World to Screen).
    glm::vec2 WorldToCamera(const glm::vec2& worldPos) const;
    // Camera to World (Screen to World).
    glm::vec2 CameraToWorld(const glm::vec2& cameraPos) const;
    // World to Physics.
    b2Vec2 WorldToPhysics(const glm::vec2& worldPos) const;
    // Physics to World.
    glm::vec2 PhysicsToWorld(const b2Vec2& physicsPos) const;
    // Camera to Physics.
    b2Vec2 CameraToPhysics(const glm::vec2& cameraPos) const;
    // Physics to Camera.
    glm::vec2 PhysicsToCamera(const b2Vec2& physicsPos) const;
};