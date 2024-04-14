#pragma once
#include <memory>
#include <utils/RAII/box2d_RAII.h>
#include <utils/box2d/box2d_body_options.h>

struct PhysicsComponent
{
    std::shared_ptr<Box2dObjectRAII> bodyRAII; // Used also for the rendering to retrieve angle and position.
    Box2dBodyOptions options;
};

struct CollisionDisableTimerComponent
{
    float timeToDisableCollision = 1.1f; // Time in seconds to disable collision.
};

struct CollisionDisableHitCountComponent
{
    size_t hitCount = 0; // Number of hits before the collision is disabled.
};

// May attach to the another entity.
struct StickyComponent
{
    bool isSticked = false; // Is the entity sticked.
};
