#pragma once
#include <box2d/box2d.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <utils/RAII/box2d_RAII.h>
#include <utils/box2d_body_options.h>
#include <utils/coordinates_transformer.h>

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
public:
    std::shared_ptr<Box2dObjectRAII> CreatePhysicsBody(
        entt::entity entity, const glm::vec2& posWorld, const glm::vec2& sizeWorld, const Box2dBodyOptions& options);
public: // Create empty physics body.
    b2Body* CreatePhysicsBodyWithNoShape(entt::entity entity, const glm::vec2& posWorld, b2BodyType bodyType);
    b2Body* CreatePhysicsBodyWithNoShape(entt::entity entity, const glm::vec2& posWorld, Box2dBodyOptions options);
public: // Create/update fixture shape for the entity.
    void CreateFixtureShapeForTheEntity(entt::entity entity, const glm::vec2& sizeWorld, Box2dBodyOptions options);
    void UpdateFixtureShapeSizeForTheEntity(entt::entity entity, const glm::vec2& sizeWorld);
private: // Add simple fixtures to the body.
    void AddBoxFixtureToBody(b2Body* body, b2FixtureDef& fixtureDef, const glm::vec2& sizeWorld);
    void AddCircleFixtureToBody(b2Body* body, b2FixtureDef& fixtureDef, const glm::vec2& sizeWorld);
    void AddVerticalCapsuleFixtureToBody(b2Body* body, b2FixtureDef& fixtureDef, const glm::vec2& sizeWorld);
    void AddThinSensorBelowTheBody(b2Body* body, const glm::vec2& sizeWorld);
private: // Get fixture def.
    b2FixtureDef GetFixtureWithOptions(const Box2dBodyOptions::Fixture& options);
private:
    void RemoveAllFixturesFromTheBody(b2Body* body);
    void CreateFixtureShapeForTheBody(
        b2Body* body, b2FixtureDef& fixtureDef, const glm::vec2& sizeWorld, Box2dBodyOptions options);
    void UpdateFixtureShapeForTheBody(b2Body* body, const glm::vec2& sizeWorld, Box2dBodyOptions options);
    void CreateFixtureShapeForTheBody(b2Body* body, const glm::vec2& sizeWorld, Box2dBodyOptions options);
};
