#pragma once
#include "utils/coordinates_transformer.h"
#include <box2d/box2d.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <utils/box2d_body_options.h>

class Box2dBodyTuner
{
    entt::registry& registry;
    CoordinatesTransformer coordinatesTransformer;
    const std::shared_ptr<b2World> physicsWorld;
public:
    Box2dBodyTuner(entt::registry& registry);
public: // Disable collisions.
    void DisableCollisionForTheBody(b2Body* body);
    void DisableCollisionForTheEntity(entt::entity entity);
public: // Set bullet flag.
    void SetBulletFlagForTheBody(b2Body* body, bool value);
    void SetBulletFlagForTheEntity(entt::entity entit, bool valuey);
public: // Create empty physics body.
    b2Body* CreatePhysicsBodyWithNoShape(entt::entity entity, const glm::vec2& posWorld, b2BodyType bodyType);
    b2Body* CreatePhysicsBodyWithNoShape(entt::entity entity, const glm::vec2& posWorld, Box2dBodyOptions options);
public: // Update fixture shape.
    void UpdateFixtureShapeForTheBody(
        b2Body* body, const glm::vec2& sizeWorld, Box2dBodyOptions::Shape shape,
        Box2dBodyOptions::Sensor sensor = Box2dBodyOptions::Sensor::NoSensor);
    void UpdateFixtureShapeForTheEntity(
        entt::entity entity, const glm::vec2& sizeWorld, Box2dBodyOptions::Shape shape,
        Box2dBodyOptions::Sensor sensor = Box2dBodyOptions::Sensor::NoSensor);
public: // Create fixture shape.
    void CreateFixtureShapeForTheBody(b2Body* body, const glm::vec2& sizeWorld, Box2dBodyOptions options);
    void CreateFixtureShapeForTheBody(
        b2Body* body, b2FixtureDef& fixtureDef, const glm::vec2& sizeWorld, Box2dBodyOptions::Shape shape,
        Box2dBodyOptions::Sensor sensor);
private: // Add simple fixtures to the body.
    void AddBoxFixtureToBody(b2Body* body, b2FixtureDef& fixtureDef, const glm::vec2& sizeWorld);
    void AddCircleFixtureToBody(b2Body* body, b2FixtureDef& fixtureDef, const glm::vec2& sizeWorld);
    void AddVerticalCapsuleFixtureToBody(b2Body* body, b2FixtureDef& fixtureDef, const glm::vec2& sizeWorld);
    void AddThinSensorBelowTheBody(b2Body* body, const glm::vec2& sizeWorld);
private: // Get fixture def.
    std::optional<b2FixtureDef> GetFixtureDefFromFirstFixtureOfTheBody(b2Body* body);
    b2FixtureDef GetFixtureWithOptions(const Box2dBodyOptions::Fixture& options);
private: // Remove fixtures.
    void RemoveAllFixturesFromTheBody(b2Body* body);
};
