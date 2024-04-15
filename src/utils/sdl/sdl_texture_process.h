#pragma once
#include <SDL.h>
#include <filesystem>
#include <memory>
#include <utils/sdl/sdl_RAII.h>

struct TextureRect
{
    std::shared_ptr<SDLTextureRAII> texture; // Pointer to the texture.
    SDL_Rect rect; // Rectangle in the texture corresponding to the tile.
};

bool IsTileInvisible(SDL_Surface* surface, const SDL_Rect& miniTextureSrcRect);

// TileId is 1-based. Tiled uses 1-based indexing.
SDL_Rect CalculateSrcRect(int tileId, int tileWidth, int tileHeight, std::shared_ptr<SDLTextureRAII> texture);

// Function to get the visible rectangle of a surface in coordinates of the surface.
SDL_Rect GetVisibleRectInSurfaceCoordinates(SDL_Surface* surface, const SDL_Rect& textureSrcRect);

// Function to get the visible rectangle of a surface in coordinates of the srcRect.
SDL_Rect GetVisibleRectInSrcRectCoordinates(SDL_Surface* surface, const SDL_Rect& textureSrcRect);

namespace details
{
std::shared_ptr<SDLTextureRAII> LoadTexture(SDL_Renderer* renderer, const std::filesystem::path& imagePath);

// Loads a surface with streaming access. This can be useful to determine if a tile is invisible.
std::shared_ptr<SDLSurfaceRAII> LoadSurfaceWithStreamingAccess(const std::filesystem::path& imagePath);

} // namespace details
