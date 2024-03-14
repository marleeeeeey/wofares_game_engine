#pragma once
#include <box2d/box2d.h>
#include <entt/entt.hpp>

class PhysicsBodyTuner
{
    entt::registry& registry;
public:
    PhysicsBodyTuner(entt::registry& registry);
public: // Disable collisions.
    static void DisableCollisionForTheBody(b2Body* body);
    void DisableCollisionForTheEntity(entt::entity entity);
    static void SetBulletFlagForTheBody(b2Body* body, bool value);
    void SetBulletFlagForTheEntity(entt::entity entit, bool valuey);
};
