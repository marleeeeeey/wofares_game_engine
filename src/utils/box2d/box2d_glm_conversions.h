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

template <typename Vec, typename Scalar, typename = std::enable_if_t<std::is_arithmetic_v<Scalar>>>
auto operator*(const Vec& vec, Scalar scalar) -> decltype(Vec(vec.x * scalar, vec.y* scalar), Vec())
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
