#include "box2d_body_creator.h"

std::shared_ptr<Box2dObjectRAII> CreateStaticPhysicsBody(
    entt::entity entity, const CoordinatesTransformer& coordinatesTransformer, std::shared_ptr<b2World> physicsWorld,
    const glm::vec2& sdlPos, const glm::vec2& sdlSize)
{
    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody;
    b2Vec2 physicalPos = coordinatesTransformer.WorldToPhysics(sdlPos);
    bodyDef.position.Set(physicalPos.x, physicalPos.y);
    b2Body* body = physicsWorld->CreateBody(&bodyDef);

    // Set the entity to the Box2D body user data. It will be used to get the entity from the Box2D body.
    body->GetUserData().pointer = static_cast<uintptr_t>(entity);

    b2PolygonShape shape;
    b2Vec2 physicalSize = coordinatesTransformer.WorldToPhysics(sdlSize);
    shape.SetAsBox(physicalSize.x / 2.0, physicalSize.y / 2.0);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    fixtureDef.density = 10.0f; // Density to calculate mass
    fixtureDef.friction = 0.3f; // Friction to apply to the body
    fixtureDef.restitution = 0.2f; // Restitution to apply to the body
    body->CreateFixture(&fixtureDef);

    return std::make_shared<Box2dObjectRAII>(body, physicsWorld);
}

std::shared_ptr<Box2dObjectRAII> CreateDynamicPhysicsBody(
    entt::entity entity, const CoordinatesTransformer& coordinatesTransformer, std::shared_ptr<b2World> physicsWorld,
    const glm::vec2& sdlPos, const glm::vec2& sdlSize)
{
    auto staticBody = CreateStaticPhysicsBody(entity, coordinatesTransformer, physicsWorld, sdlPos, sdlSize);
    staticBody->GetBody()->SetType(b2_dynamicBody);
    return staticBody;
}
