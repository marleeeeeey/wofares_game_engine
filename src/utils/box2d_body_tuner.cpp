#include "box2d_body_tuner.h"
#include <ecs/components/physics_components.h>

Box2dBodyTuner::Box2dBodyTuner(entt::registry& registry)
  : registry(registry), coordinatesTransformer(registry),
    physicsWorld(registry.get<GameOptions>(registry.view<GameOptions>().front()).physicsWorld)
{}

PhysicsComponent& Box2dBodyTuner::CreatePhysicsComponent(
    entt::entity entity, const glm::vec2& posWorld, const Box2dBodyOptions& options)
{
    b2Body* body = CreatePhysicsBodyWithNoShape(entity, posWorld);
    auto box2dObjectRAII = std::make_shared<Box2dObjectRAII>(body, physicsWorld);
    PhysicsComponent& physicsComponent = registry.emplace<PhysicsComponent>(entity, box2dObjectRAII, options);

    ApplyOption(entity, options.fixture);
    ApplyOption(entity, options.shape);
    ApplyOption(entity, options.sensor);
    ApplyOption(entity, options.dynamic);
    ApplyOption(entity, options.anglePolicy);
    ApplyOption(entity, options.collisionPolicy);
    ApplyOption(entity, options.bulletPolicy);
    ApplyOption(entity, options.hitbox);

    return physicsComponent;
}

PhysicsComponent& Box2dBodyTuner::GetPhysicsComponent(entt::entity entity)
{
    return registry.get<PhysicsComponent>(entity);
};

// ************************************* Options setters. *************************************

void Box2dBodyTuner::ApplyOption(entt::entity entity, const Box2dBodyOptions::Fixture& fixtureOptions)
{
    auto& physicsComponent = GetPhysicsComponent(entity);
    auto body = physicsComponent.bodyRAII->GetBody();
    physicsComponent.options.fixture = fixtureOptions;

    b2Fixture* fixture = body->GetFixtureList();
    while (fixture != nullptr)
    {
        fixture->SetDensity(fixtureOptions.density);
        fixture->SetFriction(fixtureOptions.friction);
        fixture->SetRestitution(fixtureOptions.restitution);
        fixture = fixture->GetNext();
    }
};

void Box2dBodyTuner::ApplyOption(entt::entity entity, const Box2dBodyOptions::Shape& option)
{
    auto& physicsComponent = GetPhysicsComponent(entity);
    auto body = physicsComponent.bodyRAII->GetBody();
    physicsComponent.options.shape = option;

    RemoveAllFixturesExceptSensorsFromTheBody(body);

    auto fixtureDef = CalcFixtureDefFromOptions(physicsComponent.options.fixture);

    if (option == Box2dBodyOptions::Shape::Box)
        AddBoxFixtureToBody(body, fixtureDef, physicsComponent.options.hitbox.sizeWorld);
    else if (option == Box2dBodyOptions::Shape::Circle)
        AddCircleFixtureToBody(body, fixtureDef, physicsComponent.options.hitbox.sizeWorld);
    else if (option == Box2dBodyOptions::Shape::Capsule)
        AddVerticalCapsuleFixtureToBody(body, fixtureDef, physicsComponent.options.hitbox.sizeWorld);
    else
        throw std::runtime_error("[ApplyOption] Unknown shape type");
};

void Box2dBodyTuner::ApplyOption(entt::entity entity, const Box2dBodyOptions::Sensor& option)
{
    auto& physicsComponent = GetPhysicsComponent(entity);
    auto body = physicsComponent.bodyRAII->GetBody();
    physicsComponent.options.sensor = option;

    RemoveAllSensorsFromTheBody(body);

    if (option == Box2dBodyOptions::Sensor::ThinSensorBelow)
        AddThinSensorBelowTheBody(body, physicsComponent.options.hitbox.sizeWorld);
    else if (option == Box2dBodyOptions::Sensor::NoSensor)
        return;
    else
        throw std::runtime_error("[ApplyOption] Unknown sensor type");
};

void Box2dBodyTuner::ApplyOption(entt::entity entity, const Box2dBodyOptions::DynamicOption& option)
{
    auto& physicsComponent = GetPhysicsComponent(entity);
    auto body = physicsComponent.bodyRAII->GetBody();
    physicsComponent.options.dynamic = option;
    body->SetType(option == Box2dBodyOptions::DynamicOption::Dynamic ? b2_dynamicBody : b2_staticBody);
};

void Box2dBodyTuner::ApplyOption(entt::entity entity, const Box2dBodyOptions::AnglePolicy& option)
{
    auto& physicsComponent = GetPhysicsComponent(entity);
    auto body = physicsComponent.bodyRAII->GetBody();
    physicsComponent.options.anglePolicy = option;

    if (option == Box2dBodyOptions::AnglePolicy::Dynamic)
        body->SetFixedRotation(false);
    else if (option == Box2dBodyOptions::AnglePolicy::Fixed)
        body->SetFixedRotation(true);
    else if (option == Box2dBodyOptions::AnglePolicy::VelocityDirection)
        // Do nothing. The angle will be set in the physics system.
        return;
    else
        throw std::runtime_error("[ApplyOption] Unknown angle policy");
};

void Box2dBodyTuner::ApplyOption(entt::entity entity, const Box2dBodyOptions::CollisionPolicy& option)
{
    auto& physicsComponent = GetPhysicsComponent(entity);
    auto body = physicsComponent.bodyRAII->GetBody();
    physicsComponent.options.collisionPolicy = option;

    b2Fixture* fixture = body->GetFixtureList();
    while (fixture != nullptr)
    {
        b2Filter filter = fixture->GetFilterData();

        if (option == Box2dBodyOptions::CollisionPolicy::NoCollision)
        {
            filter.categoryBits = 0x0000; // Mask to ignore all collisions.
        }
        else if (option == Box2dBodyOptions::CollisionPolicy::CollideWithAll)
        {
            filter.categoryBits = 0x0001; // Mask to collide with all.
        }
        else
        {
            throw std::runtime_error("[ApplyOption] Unknown collision policy");
        }

        fixture->SetFilterData(filter);
        fixture = fixture->GetNext();
    }
};

void Box2dBodyTuner::ApplyOption(entt::entity entity, const Box2dBodyOptions::BulletPolicy& option)
{
    auto& physicsComponent = GetPhysicsComponent(entity);
    auto body = physicsComponent.bodyRAII->GetBody();
    physicsComponent.options.bulletPolicy = option;

    body->SetBullet(option == Box2dBodyOptions::BulletPolicy::Bullet);
};

void Box2dBodyTuner::ApplyOption(entt::entity entity, const Box2dBodyOptions::Hitbox& hitbox)
{
    auto& physicsComponent = GetPhysicsComponent(entity);
    auto& objectHitbox = physicsComponent.options.hitbox;

    if (objectHitbox.sizeWorld == hitbox.sizeWorld)
        return;

    objectHitbox = hitbox;

    ApplyOption(entity, physicsComponent.options.shape);
};

// ************************************* Create empty physics body. *************************************

b2Body* Box2dBodyTuner::CreatePhysicsBodyWithNoShape(entt::entity entity, const glm::vec2& posWorld)
{
    b2BodyDef bodyDef;
    b2Vec2 posPhysics = coordinatesTransformer.WorldToPhysics(posWorld);
    bodyDef.position.Set(posPhysics.x, posPhysics.y);
    b2Body* body = physicsWorld->CreateBody(&bodyDef);

    // Set the entity to the Box2D body user data. It will be used to get the entity from the Box2D body.
    body->GetUserData().pointer = static_cast<uintptr_t>(entity);
    return body;
}

// ************************************* Add simple fixtures to the body. *************************************

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

// ************************************* Remove fixtures from the body. *************************************

void Box2dBodyTuner::RemoveAllFixturesExceptSensorsFromTheBody(b2Body* body)
{
    b2Fixture* fixture = body->GetFixtureList();
    while (fixture != nullptr)
    {
        if (fixture->IsSensor())
        {
            fixture = fixture->GetNext();
            continue;
        }

        b2Fixture* nextFixture = fixture->GetNext();
        body->DestroyFixture(fixture);
        fixture = nextFixture;
    }
};

void Box2dBodyTuner::RemoveAllSensorsFromTheBody(b2Body* body)
{
    b2Fixture* fixture = body->GetFixtureList();
    while (fixture != nullptr)
    {
        if (!fixture->IsSensor())
        {
            fixture = fixture->GetNext();
            continue;
        }

        b2Fixture* nextFixture = fixture->GetNext();
        body->DestroyFixture(fixture);
        fixture = nextFixture;
    }
};

// ************************************* Get fixture def. *************************************

b2FixtureDef Box2dBodyTuner::CalcFixtureDefFromOptions(const Box2dBodyOptions::Fixture& options)
{
    b2FixtureDef fixtureDef;
    fixtureDef.density = options.density;
    fixtureDef.friction = options.friction;
    fixtureDef.restitution = options.restitution;
    return fixtureDef;
};
