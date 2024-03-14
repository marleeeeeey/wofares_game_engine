#include "box2d_body_settings_helper.h"
#include <ecs/components/game_components.h>

Box2dBodySettingsHelper::Box2dBodySettingsHelper(entt::registry& registry) : registry(registry)
{}

void Box2dBodySettingsHelper::DisableCollisionForTheBody(b2Body* body)
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

void Box2dBodySettingsHelper::DisableCollisionForTheEntity(entt::entity entity)
{
    auto physicsInfo = registry.try_get<PhysicsInfo>(entity);
    if (physicsInfo)
    {
        auto body = physicsInfo->bodyRAII->GetBody();
        DisableCollisionForTheBody(body);
    }
};