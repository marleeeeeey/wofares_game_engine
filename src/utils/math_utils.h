#pragma once
#include <SDL.h>
#include <algorithm>
#include <box2d/box2d.h>

namespace utils
{
float CaclDistance(const b2Vec2& a, const b2Vec2& b);

// Returns a new Vec2 whose coordinates represent the minimum values of x and y from two provided vectors.
template <typename Vec2>
Vec2 Vec2Min(const Vec2& a, const Vec2& b)
{
    return Vec2(std::min(a.x, b.x), std::min(a.y, b.y));
}

// Returns a new Vec2 whose coordinates represent the maximum values of x and y from two provided vectors.
template <typename Vec2>
Vec2 Vec2Max(const Vec2& a, const Vec2& b)
{
    return Vec2(std::max(a.x, b.x), std::max(a.y, b.y));
}

template <typename Point, typename Bounds>
bool IsPointInsideBounds(const Point& point, const Bounds& bounds)
{
    return point.x >= bounds.min.x && point.x <= bounds.max.x && point.y >= bounds.min.y && point.y <= bounds.max.y;
}

} // namespace utils