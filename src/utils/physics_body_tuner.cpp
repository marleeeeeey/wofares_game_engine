#include "physics_body_tuner.h"
#include <ecs/components/game_components.h>

PhysicsBodyTuner::PhysicsBodyTuner(entt::registry& registry) : registry(registry)
{}

void PhysicsBodyTuner::DisableCollisionForTheBody(b2Body* body)
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

void PhysicsBodyTuner::DisableCollisionForTheEntity(entt::entity entity)
{
    auto physicsInfo = registry.try_get<PhysicsInfo>(entity);
    if (physicsInfo)
    {
        auto body = physicsInfo->bodyRAII->GetBody();
        DisableCollisionForTheBody(body);
    }
};

void PhysicsBodyTuner::SetBulletFlagForTheBody(b2Body* body, bool value)
{
    body->SetBullet(value);
}

void PhysicsBodyTuner::SetBulletFlagForTheEntity(entt::entity entity, bool value)
{
    auto physicsInfo = registry.try_get<PhysicsInfo>(entity);
    if (physicsInfo)
    {
        auto body = physicsInfo->bodyRAII->GetBody();
        SetBulletFlagForTheBody(body, value);
    }
}