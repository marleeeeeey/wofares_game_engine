#include "sdl_texture_process.h"
#include <SDL_image.h>
#include <SDL_pixels.h>
#include <SDL_surface.h>
#include <utils/RAII/sdl_RAII.h>
#include <utils/logger.h>


bool IsTileInvisible(SDL_Surface* surface, const SDL_Rect& miniTextureSrcRect)
{
    if (!surface)
        throw std::runtime_error("[IsTileInvisible] Surface is NULL");

    SDLSurfaceLockRAII lock(surface);

    Uint32* pixels = static_cast<Uint32*>(surface->pixels);
    int pitch = surface->pitch;

    for (int row = 0; row < miniTextureSrcRect.h; ++row)
    {
        for (int col = 0; col < miniTextureSrcRect.w; ++col)
        {
            Uint32 pixel = pixels[(miniTextureSrcRect.y + row) * (pitch / 4) + (miniTextureSrcRect.x + col)];
            Uint8 alpha = (pixel >> surface->format->Ashift) & 0xFF;
            if (alpha > 0)
            {
                return false;
            }
        }
    }

    return true;
}

SDL_Rect CalculateSrcRect(int tileId, int tileWidth, int tileHeight, std::shared_ptr<SDLTextureRAII> texture)
{
    int textureWidth, textureHeight;
    SDL_QueryTexture(texture->get(), nullptr, nullptr, &textureWidth, &textureHeight);

    int tilesPerRow = textureWidth / tileWidth;
    tileId -= 1; // Adjust tileId to match 0-based indexing. Tiled uses 1-based indexing.

    SDL_Rect srcRect;
    srcRect.x = (tileId % tilesPerRow) * tileWidth;
    srcRect.y = (tileId / tilesPerRow) * tileHeight;
    srcRect.w = tileWidth;
    srcRect.h = tileHeight;

    return srcRect;
}

SDL_Rect GetVisibleRectInSurfaceCoordinates(SDL_Surface* surface, const SDL_Rect& textureSrcRect)
{
    if (!surface)
        throw std::runtime_error("[GetVisibleRect] Surface is NULL");

    SDLSurfaceLockRAII lock(surface);
    Uint32* pixels = static_cast<Uint32*>(surface->pixels);
    int pitch = surface->pitch / 4; // pitch is in bytes, so divide by 4 to get the number of pixels.

    int minX = std::numeric_limits<int>::max();
    int minY = std::numeric_limits<int>::max();
    int maxX = 0;
    int maxY = 0;

    for (int row = 0; row < textureSrcRect.h; ++row)
    {
        for (int col = 0; col < textureSrcRect.w; ++col)
        {
            Uint32 pixel = pixels[(textureSrcRect.y + row) * pitch + (textureSrcRect.x + col)];
            Uint8 alpha = (pixel >> surface->format->Ashift) & 0xFF;

            if (alpha > 0)
            {
                minX = std::min(minX, col + textureSrcRect.x);
                maxX = std::max(maxX, col + textureSrcRect.x);
                minY = std::min(minY, row + textureSrcRect.y);
                maxY = std::max(maxY, row + textureSrcRect.y);
            }
        }
    }

    if (minX == std::numeric_limits<int>::max())
    {
        // No visible pixels found.
        return {0, 0, 0, 0};
    }

    SDL_Rect visibleRect = {minX, minY, maxX - minX + 1, maxY - minY + 1};
    return visibleRect;
}

SDL_Rect GetVisibleRectInSrcRectCoordinates(SDL_Surface* surface, const SDL_Rect& textureSrcRect)
{
    SDL_Rect visibleRect = GetVisibleRectInSurfaceCoordinates(surface, textureSrcRect);
    visibleRect.x -= textureSrcRect.x;
    visibleRect.y -= textureSrcRect.y;
    return visibleRect;
}

namespace details
{

std::shared_ptr<SDLTextureRAII> LoadTexture(SDL_Renderer* renderer, const std::filesystem::path& imagePath)
{
    SDL_Texture* texture = IMG_LoadTexture(renderer, imagePath.string().c_str());

    if (texture == nullptr)
        throw std::runtime_error(MY_FMT("Failed to load texture: {}", imagePath.string()));

    return std::make_shared<SDLTextureRAII>(texture);
}

SDL_Surface* ConvertSurfaceFormat(SDL_Surface* srcSurface, Uint32 toFormatEnum)
{
    if (!srcSurface)
        throw std::runtime_error("[ConvertSurfaceFormat] Source surface is NULL");

    SDLPixelFormatRAII toFormat = SDL_AllocFormat(toFormatEnum);

    // Create a new surface with the desired format.
    SDL_Surface* convertedSurface = SDL_ConvertSurface(srcSurface, toFormat.get(), 0);
    if (!convertedSurface)
        throw std::runtime_error(MY_FMT("[ConvertSurfaceFormat] Failed to convert surface: {}", SDL_GetError()));

    return convertedSurface;
}

std::shared_ptr<SDLSurfaceRAII> LoadSurfaceWithStreamingAccess(const std::filesystem::path& imagePath)
{
    std::string imagePathStr = imagePath.string();

    // Step 1. Load image into SDL_Surface.
    SDLSurfaceRAII surface = IMG_Load(imagePathStr.c_str());

    // Convert surface to target format if necessary.
    auto targetFormat = SDL_PIXELFORMAT_ABGR8888;
    if (surface.get()->format->format != targetFormat)
    {
        MY_LOG(warn, "Original surface format: {}", SDL_GetPixelFormatName(surface.get()->format->format));
        surface = ConvertSurfaceFormat(surface.get(), targetFormat);
        MY_LOG(warn, "Converted surface format: {}", SDL_GetPixelFormatName(surface.get()->format->format));
    }
    else
    {
        MY_LOG(debug, "Surface format: {}", SDL_GetPixelFormatName(surface.get()->format->format));
    }

    return std::make_shared<SDLSurfaceRAII>(std::move(surface));
}

} // namespace details