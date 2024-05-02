#pragma once
#include <memory>
#include <utils/box2d/box2d_RAII.h>
#include <utils/box2d/box2d_body_options.h>

struct PhysicsComponent
{
    std::shared_ptr<Box2dObjectRAII> bodyRAII; // Used also for the rendering to retrieve angle and position.
    Box2dBodyOptions options;
};

struct HitCountComponent
{
    size_t hitCount = 0; // Number of hits before the collision is disabled.
};

// May attach to the another entity.
struct StickyComponent
{
    bool isSticked = false; // Is the entity sticked.
};

struct ExplostionParticlesComponent
{};

struct PixeledTileComponent
{};

///// Pair DestructibleComponent and IndestructibleComponent to make the entity destructible or indestructible. ///
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct DestructibleComponent
{};

struct IndestructibleComponent
{};

///////// Pair CollidableComponent and TransparentComponent to make the entity collidable and transparent. ///////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CollidableComponent
{};

struct TransparentComponent
{};