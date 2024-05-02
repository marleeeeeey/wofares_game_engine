#pragma once
#include <box2d/box2d.h>
#include <glm/glm.hpp>

// b2Vec2 + glm::vec2 -> b2Vec2
glm::vec2 operator+(const b2Vec2& lhs, const glm::vec2& rhs);

// glm::vec2 + b2Vec2 -> glm::vec2
glm::vec2 operator+(const glm::vec2& lhs, const b2Vec2& rhs);

// b2Vec2 - glm::vec2 -> b2Vec2
glm::vec2 operator-(const b2Vec2& lhs, const glm::vec2& rhs);

// glm::vec2 - b2Vec2 -> glm::vec2
glm::vec2 operator-(const glm::vec2& lhs, const b2Vec2& rhs);
