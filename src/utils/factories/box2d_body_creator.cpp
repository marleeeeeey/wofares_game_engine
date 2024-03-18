#include "box2d_body_creator.h"
#include <box2d/b2_polygon_shape.h>

Box2dBodyCreator::Options::Options()
  : shape(Shape::Box), fixture{10.0f, 0.3f, 0.2f}, hasSensorBelowTheBody(false), isDynamic(false)
{}

Box2dBodyCreator::Box2dBodyCreator(entt::registry& registry)
  : physicsWorld(registry.get<GameOptions>(registry.view<GameOptions>().front()).physicsWorld),
    coordinatesTransformer(registry)
{}

std::shared_ptr<Box2dObjectRAII> Box2dBodyCreator::CreatePhysicsBody(
    entt::entity entity, const glm::vec2& posWorld, const glm::vec2& sizeWorld, const Options& options)
{
    // Create a Box2D body with specified position and body type.
    b2BodyType bodyType = options.isDynamic ? b2_dynamicBody : b2_staticBody;
    b2Body* body = CreatePhysicsBody(entity, posWorld, bodyType);

    // Create a fixture for the body from the options.
    b2FixtureDef fixtureDef = GetFixtureWithOptions(options.fixture);

    // Add a fixture to the body.
    if (options.shape == Options::Shape::Box)
        AddBoxFixtureToBody(body, fixtureDef, sizeWorld);
    else if (options.shape == Options::Shape::Circle)
        AddCircleFixtureToBody(body, fixtureDef, sizeWorld);
    else if (options.shape == Options::Shape::Capsule)
        AddVerticalCapsuleFixtureToBody(body, fixtureDef, sizeWorld);
    else
        throw std::runtime_error("[CreatePhysicsBody] Unknown shape type");

    // Add a thin sensor below the body if needed.
    if (options.hasSensorBelowTheBody)
        AddThinSensorBelowTheBody(body, sizeWorld);

    return std::make_shared<Box2dObjectRAII>(body, physicsWorld);
}

// ******************************** Low-x1 level methods ********************************

b2Body* Box2dBodyCreator::CreatePhysicsBody(entt::entity entity, const glm::vec2& posWorld, b2BodyType bodyType)
{
    b2BodyDef bodyDef;
    bodyDef.type = bodyType;
    b2Vec2 posPhysics = coordinatesTransformer.WorldToPhysics(posWorld);
    bodyDef.position.Set(posPhysics.x, posPhysics.y);
    b2Body* body = physicsWorld->CreateBody(&bodyDef);

    // Set the entity to the Box2D body user data. It will be used to get the entity from the Box2D body.
    body->GetUserData().pointer = static_cast<uintptr_t>(entity);
    return body;
};

void Box2dBodyCreator::AddBoxFixtureToBody(b2Body* body, b2FixtureDef& fixtureDef, const glm::vec2& sizeWorld)
{
    b2PolygonShape shape;
    b2Vec2 sizePhysics = coordinatesTransformer.WorldToPhysics(sizeWorld);
    shape.SetAsBox(sizePhysics.x / 2.0, sizePhysics.y / 2.0);
    fixtureDef.shape = &shape;
    body->CreateFixture(&fixtureDef);
};

void Box2dBodyCreator::AddCircleFixtureToBody(b2Body* body, b2FixtureDef& fixtureDef, const glm::vec2& sizeWorld)
{
    b2CircleShape shape;
    b2Vec2 sizePhysics = coordinatesTransformer.WorldToPhysics(sizeWorld);
    shape.m_radius = sizePhysics.x / 2.0;
    fixtureDef.shape = &shape;
    body->CreateFixture(&fixtureDef);
};

void Box2dBodyCreator::AddVerticalCapsuleFixtureToBody(
    b2Body* body, b2FixtureDef& fixtureDef, const glm::vec2& sizeWorld)
{
    // Convert world coordinates size to physical size for Box2D.
    b2Vec2 sizePhysics = coordinatesTransformer.WorldToPhysics(sizeWorld);
    float radius = sizePhysics.x / 2.0f; // Use width for radius to ensure the capsule fits within the given rectangle.
    float boxHeight = sizePhysics.y - 2 * radius; // Calculate the height of the central rectangular part.

    // Check if a central rectangular part is necessary.
    if (boxHeight > 0)
    {
        b2PolygonShape boxShape;
        boxShape.SetAsBox(radius, boxHeight / 2.0f, b2Vec2(0, 0), 0);
        fixtureDef.shape = &boxShape;
        body->CreateFixture(&fixtureDef);
    }

    // Add the top circular end of the capsule.
    b2CircleShape topCircle;
    topCircle.m_p.Set(0, -boxHeight / 2.0f);
    topCircle.m_radius = radius;
    fixtureDef.shape = &topCircle;
    body->CreateFixture(&fixtureDef);

    // Add the bottom circular end of the capsule.
    b2CircleShape bottomCircle;
    bottomCircle.m_p.Set(0, boxHeight / 2.0f);
    bottomCircle.m_radius = radius;
    fixtureDef.shape = &bottomCircle;
    body->CreateFixture(&fixtureDef);
}

void Box2dBodyCreator::AddThinSensorBelowTheBody(b2Body* body, const glm::vec2& sizeWorld)
{
    b2PolygonShape sensorShape;
    b2Vec2 sizePhysics = coordinatesTransformer.WorldToPhysics(sizeWorld);
    float widthFillKoef = 0.9f; // Use to prevent collision with left and right walls.
    float hw = sizePhysics.x / 2.0f * widthFillKoef;
    float hh = 0.1f;
    // Move center of polygon to the bottom of the body. Slighly above the ground.
    b2Vec2 center(0, sizePhysics.y / 2.0f);
    float angle = 0;
    sensorShape.SetAsBox(hw, hh, center, angle);

    b2FixtureDef sensorDef;
    sensorDef.shape = &sensorShape;
    sensorDef.isSensor = true;

    // Add a sensor to the body below the body.
    body->CreateFixture(&sensorDef);
};

// ******************************** Low-x2 level methods ********************************

b2FixtureDef Box2dBodyCreator::GetFixtureWithOptions(const Options::Fixture& options)
{
    b2FixtureDef fixtureDef;
    fixtureDef.density = options.density;
    fixtureDef.friction = options.friction;
    fixtureDef.restitution = options.restitution;
    return fixtureDef;
};