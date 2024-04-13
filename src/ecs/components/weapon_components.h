#pragma once

// Damage component. Used to describe the damage.
struct DamageComponent
{
    float radius = 0.5f; // Radius of the explosion in meters.
    float force = 1000.0f; // Force of the explosion in newtons.
};

// Like a bazooka projectile. Explodes when it touches a tile.
struct ExplosionOnContactComponent
{};

// Like a grenade. Explodes after a certain time.
struct ExplosionOnTimerComponent
{};
