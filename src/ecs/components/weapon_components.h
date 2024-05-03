#pragma once

// Damage component. Used to describe the damage.
struct DamageComponent
{
    float radius = 0.5f; // Radius of the explosion in meters.
    float force = 0.50f; // Force of the explosion in newtons.
};

// Like a bazooka projectile. Explodes when it touches a tile.
struct ExplosionOnContactComponent
{};

// Like a grenade. Explodes after a certain time.
struct ExplosionOnTimerComponent
{};

// TODO1: Reuse instead of weapon props.
struct FireRateComponent
{
    float fireRate = 0.0f; // Time in seconds between shots.
    float remainingFireRate = 0.0f; // Remaining time in seconds to fire the next shot.
};