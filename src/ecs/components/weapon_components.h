#pragma once

// TODO0
// ********************************* Munitions components *********************************
// Bullet, Bomb, Mine, Grenade, Missile, Shell, Torpedo, Flashbang, Smoke grenade, Fragmentation grenade, Incendiary
// bomb, Chemical bomb, Nuclear bomb, Cluster bomb, High explosive

struct BulletComponent
{};

struct GrenadeComponent
{};

// ********************************* Explosion components *********************************

// Like a grenade. Explodes after a certain time.
struct TimerExplosionComponent
{
    float timeToExplode = 3.0f; // Time in seconds before the grenade explodes
};

// Like a bazooka projectile. Explodes when it touches a tile.
struct ContactExplosionComponent
{
    float spawnSafeTime = 0.0f; // Time in seconds before the explosion is activated.
};

// Like a bomb. Explodes when a remote trigger is activated.
struct RemoteExplosionComponent
{
    bool isTriggered = false; // If true, the explosion is triggered.
};

// Like a mine. Explodes when a player touches it.
struct PlayerContactExplosionComponent
{
    float activationTime; // Time in seconds after contact with a player.
    bool isActivated = false; // Flag indicating whether the explosion timer is activated.
    float timer; // Countdown timer after activation.
};

struct ExplosionImpactComponent
{
    float radius = 0.5f; // Radius of the explosion in meters.
    float force = 1000.0f; // Force of the explosion in newtons.
};
