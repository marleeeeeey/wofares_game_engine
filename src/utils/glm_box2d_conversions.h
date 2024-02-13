#pragma once
#include <box2d/box2d.h>
#include <glm/glm.hpp>
#include <stdexcept>

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

template <typename Vec, typename Scalar>
Vec operator/(const Vec& vec, Scalar scalar)
{
    if (scalar == 0)
        throw std::runtime_error("Division by zero");
    return Vec(vec.x / scalar, vec.y / scalar);
}

glm::vec2 toGmlVec2(const b2Vec2& vec);

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