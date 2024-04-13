#pragma once
#include "utils/box2d_body_options.h"
#include <glm/glm.hpp>

enum class WeaponType
{
    Uzi,
    Shotgun,
    Bazooka,
    Grenade,
    StickTrap
};

struct WeaponProps
{
public: // Bullet.
    float bulletMass = 0.1; // Mass of the bullet, kg.
    float bulletEjectionForce = 0.1; // Force of the bullet ejection. For grenades it should be zero.
    Box2dBodyOptions::AnglePolicy bulletAnglePolicy =
        Box2dBodyOptions::AnglePolicy::VelocityDirection; // Angle policy of the bullet.
public: // Damage.
    glm::vec2 projectileSizeWorld = {10, 5}; // Size of the projectile in pixels.
    size_t damageRadiusWorld = 10; // Radius of the damage in pixels.
    float damageForce = 10; // Force of the damage.
public: // Ammo.
    size_t ammoInStorage = 100; // Current number of bullets except in the clip.
    size_t ammoInClip = 10; // Current number of bullets in the clip.
    size_t clipSize = 10; // Max number of bullets in the clip.
public: // Reload.
    float reloadTime = 5.0f; // Time in seconds to reload the weapon.
    float remainingReloadTime = 0.0f; // Remaining time in seconds to reload the weapon.
public: // Fire rate.
    float fireRate = 0.5f; // Time in seconds between shots.
    float remainingFireRate = 0.0f; // Remaining time in seconds to fire the next shot.
};
