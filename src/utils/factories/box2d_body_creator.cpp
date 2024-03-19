#include "box2d_body_creator.h"
#include "ecs/components/physics_components.h"
#include "utils/box2d_body_options.h"
#include <box2d/b2_polygon_shape.h>

Box2dBodyCreator::Box2dBodyCreator(entt::registry& registry)
  : physicsWorld(registry.get<GameOptions>(registry.view<GameOptions>().front()).physicsWorld), bodyTuner(registry)
{}

PhysicsComponent& Box2dBodyCreator::CreatePhysicsBody(
    entt::entity entity, const glm::vec2& posWorld, const Box2dBodyOptions& options)
{
    return bodyTuner.CreatePhysicsComponent(entity, posWorld, options);
}

PhysicsComponent& Box2dBodyCreator::CreatePhysicsBody(
    entt::entity entity, const glm::vec2& posWorld, const glm::vec2& sizeWorld, Box2dBodyOptions options)
{
    options.hitbox.sizeWorld = sizeWorld;
    return bodyTuner.CreatePhysicsComponent(entity, posWorld, options);
};