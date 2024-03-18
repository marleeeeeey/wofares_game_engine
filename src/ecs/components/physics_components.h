#pragma once
#include <memory>
#include <utils/RAII/box2d_RAII.h>

struct PhysicsComponent
{
    // Used also for the rendering to retrieve angle and position.
    std::shared_ptr<Box2dObjectRAII> bodyRAII;
};

struct CollisionDisableTimerComponent
{
    float timeToDisableCollision = 1.1f; // Time in seconds to disable collision.
};

struct CollisionDisableHitCountComponent
{
    size_t hitCount = 0; // Number of hits before the collision is disabled.
};