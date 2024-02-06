#include "collisions.h"

AABB СalculateAABB(const Position& position, const SizeComponent& size)
{
    AABB aabb;
    aabb.min = position.value;
    aabb.max = position.value + size.value;
    return aabb;
}

bool CheckAABBCollision(const AABB& a, const AABB& b)
{
    if (a.max.x < b.min.x || a.min.x > b.max.x)
        return false;
    if (a.max.y < b.min.y || a.min.y > b.max.y)
        return false;
    return true;
}
