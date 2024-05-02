#include "box2d_glm_operators.h"

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
