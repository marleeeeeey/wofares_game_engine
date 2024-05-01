#pragma once
#include <stdexcept>
#include <type_traits>

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
