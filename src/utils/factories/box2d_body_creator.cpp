#include "box2d_body_creator.h"
#include "utils/box2d_body_options.h"
#include <box2d/b2_polygon_shape.h>

Box2dBodyCreator::Box2dBodyCreator(entt::registry& registry)
  : physicsWorld(registry.get<GameOptions>(registry.view<GameOptions>().front()).physicsWorld), bodyTuner(registry)
{}

std::shared_ptr<Box2dObjectRAII> Box2dBodyCreator::CreatePhysicsBody(
    entt::entity entity, const glm::vec2& posWorld, const glm::vec2& sizeWorld, const Box2dBodyOptions& options)
{
    return bodyTuner.CreatePhysicsBody(entity, posWorld, sizeWorld, options);
}
