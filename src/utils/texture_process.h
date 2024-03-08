#pragma once
#include <SDL.h>
#include <filesystem>
#include <memory>
#include <utils/sdl_RAII.h>

bool IsTileInvisible(SDL_Surface* surface, const SDL_Rect& miniTextureSrcRect);

// TileId is 1-based. Tiled uses 1-based indexing.
SDL_Rect CalculateSrcRect(int tileId, int tileWidth, int tileHeight, std::shared_ptr<SDLTextureRAII> texture);

// Split rect into m x n smaller rects.
std::vector<SDL_Rect> SplitRect(const SDL_Rect& rect, int m, int n);

// Function to divide an SDL_Rect into smaller rectangles based on cell size.
std::vector<SDL_Rect> DivideRectByCellSize(const SDL_Rect& rect, const SDL_Point& cellSize);

namespace details
{
std::shared_ptr<SDLTextureRAII> LoadTexture(SDL_Renderer* renderer, const std::filesystem::path& imagePath);

// Loads a surface with streaming access. This can be useful to determine if a tile is invisible.
std::shared_ptr<SDLSurfaceRAII> LoadSurfaceWithStreamingAccess(
    SDL_Renderer* renderer, const std::filesystem::path& imagePath);

} // namespace details
