#include "box2d_body_tuner.h"
#include "my_common_cpp_utils/logger.h"
#include "my_common_cpp_utils/math_utils.h"
#include <ecs/components/physics_components.h>

Box2dBodyTuner::Box2dBodyTuner(entt::registry& registry)
  : registry(registry), coordinatesTransformer(registry),
    physicsWorld(registry.get<GameOptions>(registry.view<GameOptions>().front()).physicsWorld)
{}

void Box2dBodyTuner::DisableCollisionForTheBody(b2Body* body)
{
    b2Fixture* fixture = body->GetFixtureList();
    while (fixture != nullptr)
    {
        b2Filter filter = fixture->GetFilterData();
        filter.maskBits = 0x0000; // Mask to ignore all collisions.
        fixture->SetFilterData(filter);
        fixture = fixture->GetNext();
    }
};

void Box2dBodyTuner::DisableCollisionForTheEntity(entt::entity entity)
{
    auto physicsInfo = registry.try_get<PhysicsComponent>(entity);
    if (!physicsInfo)
        return;

    auto body = physicsInfo->bodyRAII->GetBody();
    DisableCollisionForTheBody(body);
};

void Box2dBodyTuner::SetBulletFlagForTheBody(b2Body* body, bool value)
{
    body->SetBullet(value);
}

void Box2dBodyTuner::SetBulletFlagForTheEntity(entt::entity entity, bool value)
{
    auto physicsInfo = registry.try_get<PhysicsComponent>(entity);
    if (!physicsInfo)
        return;

    auto body = physicsInfo->bodyRAII->GetBody();
    SetBulletFlagForTheBody(body, value);
}

void Box2dBodyTuner::UpdateFixtureShapeForTheBody(b2Body* body, const glm::vec2& sizeWorld, Box2dBodyOptions options)
{
    RemoveAllFixturesFromTheBody(body);
    auto fixtureDef = GetFixtureWithOptions(options.fixture);
    CreateFixtureShapeForTheBody(body, fixtureDef, sizeWorld, options);
    body->ResetMassData();
};

void Box2dBodyTuner::CreateFixtureShapeForTheBody(b2Body* body, const glm::vec2& sizeWorld, Box2dBodyOptions options)
{
    b2FixtureDef fixtureDef = GetFixtureWithOptions(options.fixture);
    CreateFixtureShapeForTheBody(body, fixtureDef, sizeWorld, options);
};

void Box2dBodyTuner::CreateFixtureShapeForTheEntity(
    entt::entity entity, const glm::vec2& sizeWorld, Box2dBodyOptions options)
{
    auto physicsInfo = registry.try_get<PhysicsComponent>(entity);
    if (!physicsInfo)
        return;

    auto body = physicsInfo->bodyRAII->GetBody();
    physicsInfo->options = options;
    physicsInfo->sizeWorld = sizeWorld;
    CreateFixtureShapeForTheBody(body, sizeWorld, options);
};

void Box2dBodyTuner::UpdateFixtureShapeSizeForTheEntity(entt::entity entity, const glm::vec2& sizeWorld)
{
    auto physicsInfo = registry.try_get<PhysicsComponent>(entity);
    if (!physicsInfo)
        return;

    auto body = physicsInfo->bodyRAII->GetBody();

    if (physicsInfo->sizeWorld == sizeWorld)
        return;

    physicsInfo->sizeWorld = sizeWorld;

    // TODO4: This method makes the body to lose it's contacts for a short period of time.
    // That is why this method should be called as rarely as possible.
    UpdateFixtureShapeForTheBody(body, sizeWorld, physicsInfo->options);
};

void Box2dBodyTuner::CreateFixtureShapeForTheBody(
    b2Body* body, b2FixtureDef& fixtureDef, const glm::vec2& sizeWorld, Box2dBodyOptions options)
{
    if (options.shape == Box2dBodyOptions::Shape::Box)
        AddBoxFixtureToBody(body, fixtureDef, sizeWorld);
    else if (options.shape == Box2dBodyOptions::Shape::Circle)
        AddCircleFixtureToBody(body, fixtureDef, sizeWorld);
    else if (options.shape == Box2dBodyOptions::Shape::Capsule)
        AddVerticalCapsuleFixtureToBody(body, fixtureDef, sizeWorld);
    else
        throw std::runtime_error("[CreateHitboxShapeForTheBody] Unknown shape type");

    if (options.sensor == Box2dBodyOptions::Sensor::ThinSensorBelow)
        AddThinSensorBelowTheBody(body, sizeWorld);
};

void Box2dBodyTuner::AddBoxFixtureToBody(b2Body* body, b2FixtureDef& fixtureDef, const glm::vec2& sizeWorld)
{
    b2PolygonShape shape;
    b2Vec2 sizePhysics = coordinatesTransformer.WorldToPhysics(sizeWorld);
    shape.SetAsBox(sizePhysics.x / 2.0, sizePhysics.y / 2.0);
    fixtureDef.shape = &shape;
    body->CreateFixture(&fixtureDef);
};

void Box2dBodyTuner::AddCircleFixtureToBody(b2Body* body, b2FixtureDef& fixtureDef, const glm::vec2& sizeWorld)
{
    b2CircleShape shape;
    b2Vec2 sizePhysics = coordinatesTransformer.WorldToPhysics(sizeWorld);
    shape.m_radius = sizePhysics.x / 2.0;
    fixtureDef.shape = &shape;
    body->CreateFixture(&fixtureDef);
};

void Box2dBodyTuner::AddVerticalCapsuleFixtureToBody(b2Body* body, b2FixtureDef& fixtureDef, const glm::vec2& sizeWorld)
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

void Box2dBodyTuner::AddThinSensorBelowTheBody(b2Body* body, const glm::vec2& sizeWorld)
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

void Box2dBodyTuner::RemoveAllFixturesFromTheBody(b2Body* body)
{
    b2Fixture* fixture = body->GetFixtureList();
    while (fixture != nullptr)
    {
        b2Fixture* nextFixture = fixture->GetNext();
        body->DestroyFixture(fixture);
        fixture = nextFixture;
    }
};

b2Body* Box2dBodyTuner::CreatePhysicsBodyWithNoShape(
    entt::entity entity, const glm::vec2& posWorld, b2BodyType bodyType)
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

b2FixtureDef Box2dBodyTuner::GetFixtureWithOptions(const Box2dBodyOptions::Fixture& options)
{
    b2FixtureDef fixtureDef;
    fixtureDef.density = options.density;
    fixtureDef.friction = options.friction;
    fixtureDef.restitution = options.restitution;
    return fixtureDef;
};

b2Body* Box2dBodyTuner::CreatePhysicsBodyWithNoShape(
    entt::entity entity, const glm::vec2& posWorld, Box2dBodyOptions options)
{
    b2BodyType bodyType;
    if (options.dynamic == Box2dBodyOptions::DynamicOptions::Dynamic)
    {
        bodyType = b2_dynamicBody;
    }
    else if (options.dynamic == Box2dBodyOptions::DynamicOptions::Static)
    {
        bodyType = b2_staticBody;
    }
    else
    {
        throw std::runtime_error("[CreatePhysicsBody] Unknown dynamic type");
    }

    return CreatePhysicsBodyWithNoShape(entity, posWorld, bodyType);
};

std::shared_ptr<Box2dObjectRAII> Box2dBodyTuner::CreatePhysicsBody(
    entt::entity entity, const glm::vec2& posWorld, const glm::vec2& sizeWorld, const Box2dBodyOptions& options)
{
    b2Body* body = CreatePhysicsBodyWithNoShape(entity, posWorld, options);
    CreateFixtureShapeForTheBody(body, sizeWorld, options);
    return std::make_shared<Box2dObjectRAII>(body, physicsWorld);
};