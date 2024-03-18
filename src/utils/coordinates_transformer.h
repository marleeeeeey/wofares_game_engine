#pragma once
#include <box2d/box2d.h>
#include <entt/entt.hpp>
#include <utils/game_options.h>

// There are 3 coordinate systems in the game:
// 1. World (game) coordinates.
// 2. Screen coordinates.
// 3. Physics (box2d) coordinates.
class CoordinatesTransformer
{
    const GameOptions& gameState;
    const float box2DtoWorld; // 1 meter in Box2D is XX pixels in SDL.
    const float worldToBox2D; // 1 pixel in SDL is XX meters in Box2D.
public:
    enum class Type
    {
        Position,
        Length,
    };

    CoordinatesTransformer(entt::registry& registry);
    // World to Screen.
    glm::vec2 WorldToScreen(const glm::vec2& posWorld, Type type = Type::Position) const;
    float WorldToScreen(float valueWorld) const;
    // Screen to World.
    glm::vec2 ScreenToWorld(const glm::vec2& posScreen, Type type = Type::Position) const;
    float ScreenToWorld(float valueScreen) const;
    // World to Physics.
    b2Vec2 WorldToPhysics(const glm::vec2& posWorld, Type type = Type::Position) const;
    float WorldToPhysics(float valueWorld) const;
    // Physics to World.
    glm::vec2 PhysicsToWorld(const b2Vec2& posPhysics, Type type = Type::Position) const;
    float PhysicsToWorld(float valuePhysics) const;
    // Screen to Physics.
    b2Vec2 ScreenToPhysics(const glm::vec2& posScreen, Type type = Type::Position) const;
    float ScreenToPhysics(float valueScreen) const;
    // Physics to Screen.
    glm::vec2 PhysicsToScreen(const b2Vec2& posPhysics, Type type = Type::Position) const;
    float PhysicsToScreen(float valuePhysics) const;
};