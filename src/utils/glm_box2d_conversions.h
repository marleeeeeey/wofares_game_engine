#pragma once
#include <box2d/box2d.h>
#include <glm/glm.hpp>
#include <stdexcept>
#include <type_traits>

// b2Vec2 + glm::vec2 -> b2Vec2
glm::vec2 operator+(const b2Vec2& lhs, const glm::vec2& rhs);

// glm::vec2 + b2Vec2 -> glm::vec2
glm::vec2 operator+(const glm::vec2& lhs, const b2Vec2& rhs);

// b2Vec2 - glm::vec2 -> b2Vec2
glm::vec2 operator-(const b2Vec2& lhs, const glm::vec2& rhs);

// glm::vec2 - b2Vec2 -> glm::vec2
glm::vec2 operator-(const glm::vec2& lhs, const b2Vec2& rhs);

template <typename Vec, typename Scalar>
Vec operator*(const Vec& vec, Scalar scalar)
{
    return Vec(vec.x * scalar, vec.y * scalar);
}

template <typename Vec, typename Scalar, typename = std::enable_if_t<std::is_arithmetic_v<Scalar>>>
auto operator/(const Vec& vec, Scalar scalar) -> decltype(Vec(vec.x / scalar, vec.y / scalar), Vec())
{
    if (scalar == 0)
        throw std::runtime_error("Division by zero");
    return Vec(vec.x / scalar, vec.y / scalar);
}

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

namespace utils
{
float distance(const b2Vec2& a, const b2Vec2& b);
}