#pragma once
#include <SDL.h>
#include <chrono>
#include <cstddef>
#include <glm/glm.hpp>
#include <memory>
#include <utils/sdl/sdl_RAII.h>
#include <utils/sdl/sdl_colors.h>

enum class ZOrderingType
{
    Background,
    Interiors,
    Terrain,
};

struct TileComponent
{
    glm::vec2 sizeWorld = {0, 0}; // Must be installed manually. Hard to retrieve from Box2D body.
    std::shared_ptr<SDLTextureRAII> texturePtr{}; // Pointer to the texture.
    SDL_Rect textureRect{}; // Rectangle in the texture corresponding to the tile.
    ZOrderingType zOrderingType = ZOrderingType::Terrain;
    ColorName colorName = ColorName::Blue; // Color if the texture is not available.
};

struct DebugVisualObjectComponent
{};

struct NameComponent
{
    std::string name;
};

struct CreationTimeComponent
{
    std::chrono::milliseconds creationTime = std::chrono::milliseconds(0);
};

struct MarkForTrailDebugComponent
{
    size_t trailSize = 0;
};