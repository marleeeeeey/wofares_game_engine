#pragma once
#include <ecs/components/all_components.h>
#include <glm/glm.hpp>

struct AABB
{
    glm::vec2 min; // Left top corner
    glm::vec2 max; // Right upper corner
};

AABB СalculateAABB(const Position& position, const SizeComponent& size);

bool CheckAABBCollision(const AABB& a, const AABB& b);
