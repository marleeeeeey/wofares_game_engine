#pragma once
#include <SDL.h>
#include <glm/glm.hpp>
#include <memory>
#include <utils/RAII/sdl_RAII.h>
#include <utils/sdl_colors.h>

struct RenderingComponent
{
    glm::vec2 sizeWorld = {0, 0}; // Must be installed manually. Hard to retrieve from Box2D body.
    std::shared_ptr<SDLTextureRAII> texturePtr{}; // Pointer to the texture.
    SDL_Rect textureRect{}; // Rectangle in the texture corresponding to the tile.
    ColorName colorName = ColorName::Blue; // Color if the texture is not available.
};
