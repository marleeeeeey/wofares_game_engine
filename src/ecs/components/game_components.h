#pragma once
#include <box2d/box2d.h>
#include <glm/glm.hpp>
#include <memory>
#include <utils/box2d_RAII.h>
#include <utils/sdl_RAII.h>

struct SdlSizeComponent
{
    // Must be installed manually. Because this components is not calculated from Box2D body.
    glm::vec2 value = {0, 0};
};

struct PlayerNumber
{
    size_t value = 0;
};

struct PlayersWeaponDirection
{
    glm::vec2 value = {1, 0}; // {1, 0} - look to the right, {0, 1} - look down,
};

struct TileInfo
{
    std::shared_ptr<SDLTextureRAII> texture; // Pointer to the texture.
    SDL_Rect srcRect; // Rectangle in the texture corresponding to the tile.
};

struct PhysicalBody
{
    std::shared_ptr<Box2dObjectRAII> value;
};

struct Grenade
{
    float timeToExplode = 3.0f; // Time in seconds before the grenade explodes
};

struct Bridge
{};
