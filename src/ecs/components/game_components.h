#pragma once
#include <box2d/box2d.h>
#include <glm/glm.hpp>
#include <memory>
#include <utils/box2d_RAII.h>
#include <utils/sdl_RAII.h>

// ********************************* Rendering components *********************************

struct SdlSizeComponent
{
    // Must be installed manually. Because this components is not calculated from Box2D body.
    glm::vec2 value = {0, 0};
};

struct TileInfo
{
    std::shared_ptr<SDLTextureRAII> texture; // Pointer to the texture.
    SDL_Rect srcRect; // Rectangle in the texture corresponding to the tile.
};

// ********************************* Player components *********************************

struct PlayerNumber
{
    size_t value = 0;
};

struct PlayersWeaponDirection
{
    glm::vec2 value = {1, 0}; // {1, 0} - look to the right, {0, 1} - look down,
};

// ********************************* Physics components *********************************

struct PhysicalBody
{
    std::shared_ptr<Box2dObjectRAII> value;
};

// ********************************* Weapon components *********************************

struct Grenade
{
    float timeToExplode = 3.0f; // Time in seconds before the grenade explodes
    float explosionRadius = 0.5f;
};

struct Bazooka
{
    float timeToReload = 1.0f; // Time in seconds to reload the weapon
};

// ********************************* Building components *********************************

struct Bridge
{};
