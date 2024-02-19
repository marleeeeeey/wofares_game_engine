#pragma once
#include "entt/entity/fwd.hpp"
#include <ecs/components/game_state_component.h>
#include <utils/box2d_RAII.h>
#include <utils/coordinates_transformer.h>

class Box2dBodyCreator
{
    const std::shared_ptr<b2World> physicsWorld;
    CoordinatesTransformer coordinatesTransformer;
public:
    Box2dBodyCreator(entt::registry& registry);

    // entity is used to store entity in the physics body.
    std::shared_ptr<Box2dObjectRAII> CreateStaticPhysicsBody(
        entt::entity entity, const glm::vec2& sdlPos, const glm::vec2& sdlSize);

    // entity is used to store entity in the physics body.
    std::shared_ptr<Box2dObjectRAII> CreateDynamicPhysicsBody(
        entt::entity entity, const glm::vec2& sdlPos, const glm::vec2& sdlSize);

    void AddThinSensorBelowTheBody(std::shared_ptr<Box2dObjectRAII>, const glm::vec2& sdlSize);
};