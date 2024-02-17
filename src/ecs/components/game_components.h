#pragma once
#include "utils/sdl_colors.h"
#include <box2d/box2d.h>
#include <glm/glm.hpp>
#include <memory>
#include <utils/box2d_RAII.h>
#include <utils/sdl_RAII.h>

// ********************************* Rendering components *********************************

struct RenderingInfo
{
    glm::vec2 sdlSize = {0, 0}; // Must be installed manually. Hard to retrieve from Box2D body.
    std::shared_ptr<SDLTextureRAII> texturePtr{}; // Pointer to the texture.
    SDL_Rect textureRect{}; // Rectangle in the texture corresponding to the tile.
    ColorName colorName = ColorName::Blue; // Color if the texture is not available.
};

// ********************************* Player components *********************************

struct PlayerInfo
{
    enum class Weapon
    {
        None,
        Bazooka,
        Grenade,
    };

    size_t number = 0;
    Weapon currentWeapon = Weapon::Bazooka;
    glm::vec2 weaponDirection = {1, 0};
    float timeToReload = 1.0f; // Time in seconds to reload the weapon. TODO: think how to use it.
    bool canJump = false; // Flag indicating whether the player can jump.
};

// ********************************* Physics components *********************************

struct PhysicsInfo
{
    // Used also for the rendering to retrieve angle and position.
    std::shared_ptr<Box2dObjectRAII> bodyRAII;
};

struct CollisionDisableTimerComponent
{
    float timeToDisableCollision = 2.5f; // Time in seconds to disable collision.
};

// ********************************* Weapon components *********************************

// Like a grenade. Explodes after a certain time.
struct TimerExplosionComponent
{
    float timeToExplode = 3.0f; // Time in seconds before the grenade explodes
};

// Like a bazooka projectile. Explodes when it touches a tile.
struct ContactExplosionComponent
{};

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
