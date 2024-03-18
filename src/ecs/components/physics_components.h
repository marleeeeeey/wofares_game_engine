#pragma once
#include <memory>
#include <utils/RAII/box2d_RAII.h>
#include <utils/angle_policy.h>

struct PhysicsComponent
{
    std::shared_ptr<Box2dObjectRAII> bodyRAII; // Used also for the rendering to retrieve angle and position.
    AnglePolicy anglePolicy = AnglePolicy::Dynamic;
};

struct CollisionDisableTimerComponent
{
    float timeToDisableCollision = 1.1f; // Time in seconds to disable collision.
};

struct CollisionDisableHitCountComponent
{
    size_t hitCount = 0; // Number of hits before the collision is disabled.
};