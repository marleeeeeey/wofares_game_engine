#pragma once
#include <box2d/box2d.h>
#include <entt/entt.hpp>

enum class CollisionFlags : uint16
{
    None = 0,
    Default = 1 << 0,
    Bullet = 1 << 1,
    _entt_enum_as_bitmask
};
