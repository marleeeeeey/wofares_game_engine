#pragma once
#include "entt/entity/fwd.hpp"
#include <utils/box2d_RAII.h>
#include <utils/coordinates_transformer.h>
#include <utils/game_options.h>


class Box2dBodyCreator
{
public: // Public types.
    struct Options
    {
        enum class Shape
        {
            Box,
            Capsule
        } shape;

        struct Fixture
        {
            float density; // Density to calculate mass.
            float friction; // Friction to apply to the body.
            float restitution; // Restitution to apply to the body.
        } fixture;

        bool hasSensorBelowTheBody;
        bool isDynamic;

        // Default constructor
        Options();
    };
public: // Public methods.
    Box2dBodyCreator(entt::registry& registry);
    // entity is used to store entity in the physics body.
    std::shared_ptr<Box2dObjectRAII> CreatePhysicsBody(
        entt::entity entity, const glm::vec2& sdlPos, const glm::vec2& sdlSize, const Options& options = {});
private: // Low-x1 level methods.
    b2Body* CreatePhysicsBody(entt::entity entity, const glm::vec2& sdlPos, b2BodyType bodyType);
    void AddBoxFixtureToBody(b2Body* body, b2FixtureDef& fixtureDef, const glm::vec2& sdlSize);
    void AddCapsuleFixtureToBody(b2Body* body, b2FixtureDef& fixtureDef, const glm::vec2& sdlSize);
    void AddThinSensorBelowTheBody(b2Body* body, const glm::vec2& sdlSize);
private: // Low-x2 level methods.
    b2FixtureDef GetFixtureWithOptions(const Options::Fixture& options);
private: // State.
    const std::shared_ptr<b2World> physicsWorld;
    CoordinatesTransformer coordinatesTransformer;
};