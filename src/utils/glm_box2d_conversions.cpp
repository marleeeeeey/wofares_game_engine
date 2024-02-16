#include "glm_box2d_conversions.h"
#include <cmath>

glm::vec2 operator+(const b2Vec2& lhs, const glm::vec2& rhs)
{
    return glm::vec2(lhs.x + rhs.x, lhs.y + rhs.y);
}
glm::vec2 operator+(const glm::vec2& lhs, const b2Vec2& rhs)
{
    return glm::vec2(lhs.x + rhs.x, lhs.y + rhs.y);
}
glm::vec2 operator-(const b2Vec2& lhs, const glm::vec2& rhs)
{
    return glm::vec2(lhs.x - rhs.x, lhs.y - rhs.y);
}
glm::vec2 operator-(const glm::vec2& lhs, const b2Vec2& rhs)
{
    return glm::vec2(lhs.x - rhs.x, lhs.y - rhs.y);
}

namespace utils
{
float distance(const b2Vec2& a, const b2Vec2& b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}
} // namespace utils