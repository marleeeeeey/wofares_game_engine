#pragma once
#include <SDL.h>
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

struct RenderingComponent
{
    glm::vec2 sizeWorld = {0, 0}; // Must be installed manually. Hard to retrieve from Box2D body.
    std::shared_ptr<SDLTextureRAII> texturePtr{}; // Pointer to the texture.
    SDL_Rect textureRect{}; // Rectangle in the texture corresponding to the tile.
    ZOrderingType zOrderingType = ZOrderingType::Terrain;
    ColorName colorName = ColorName::Blue; // Color if the texture is not available.
};
