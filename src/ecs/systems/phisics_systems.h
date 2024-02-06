#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>

void BoundarySystem(entt::registry& registry, const glm::vec2& windowSize);

void PhysicsSystem(entt::registry& registry, float deltaTime);
