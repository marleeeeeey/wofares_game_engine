#pragma once
#include <glm/fwd.hpp>
#include <utils/weapon.h>

struct TurretComponent
{
    bool shooting{false};
    glm::vec2 gunGirection{0.0f, -1.0f};
    WeaponProps weaponProps;
    float timeSinceLastShot{0.0f};
};