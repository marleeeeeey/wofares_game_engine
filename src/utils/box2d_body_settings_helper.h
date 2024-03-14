#pragma once
#include <box2d/box2d.h>
#include <entt/entt.hpp>

class Box2dBodySettingsHelper
{
    entt::registry& registry;
public:
    Box2dBodySettingsHelper(entt::registry& registry);
public: // Disable collisions.
    static void DisableCollisionForTheBody(b2Body* body);
    void DisableCollisionForTheEntity(entt::entity entity);
};
