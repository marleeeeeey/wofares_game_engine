#include "physics_body_creator.h"
#include <utils/globals.h>

std::shared_ptr<Box2dObjectRAII> CreateStaticPhysicsBody(
    std::shared_ptr<b2World> physicsWorld, const glm::u32vec2& sdlPos, const glm::u32vec2& sdlSize)
{
    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody;
    bodyDef.position.Set(sdlPos.x * sdlToBox2D, sdlPos.y * sdlToBox2D);
    b2Body* body = physicsWorld->CreateBody(&bodyDef);

    b2PolygonShape shape;
    shape.SetAsBox(sdlSize.x / 2.0 * sdlToBox2D, sdlSize.y / 2.0 * sdlToBox2D);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    fixtureDef.density = 10.0f; // Density to calculate mass
    fixtureDef.friction = 0.3f; // Friction to apply to the body
    body->CreateFixture(&fixtureDef);

    return std::make_shared<Box2dObjectRAII>(body, physicsWorld);
}

std::shared_ptr<Box2dObjectRAII> CreateDynamicPhysicsBody(
    std::shared_ptr<b2World> physicsWorld, const glm::u32vec2& sdlPos, const glm::u32vec2& sdlSize)
{
    auto staticBody = CreateStaticPhysicsBody(physicsWorld, sdlPos, sdlSize);
    staticBody->GetBody()->SetType(b2_dynamicBody);
    return staticBody;
}
