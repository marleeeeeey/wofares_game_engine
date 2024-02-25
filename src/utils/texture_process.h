#pragma once
#include <SDL.h>
#include <filesystem>
#include <memory>
#include <utils/sdl_RAII.h>

/**
 * Checks if the given tileset texture has an opaque pixel within the specified source rectangle.
 * Note that in order to use SDL_LockTexture and SDL_UnlockTexture, the texture must be created with
 * SDL_TEXTUREACCESS_STREAMING access.
 *
 * @param tilesetTexture The tileset texture to check.
 * @param miniTextureSrcRect The source rectangle within the tileset texture.
 * @return True if the tileset texture has an opaque pixel within the source rectangle, false otherwise.
 */
bool IsTileInvisible(std::shared_ptr<SDLTextureRAII> tilesetTexture, const SDL_Rect& miniTextureSrcRect);

SDL_Rect CalculateSrcRect(int tileId, int tileWidth, int tileHeight, std::shared_ptr<SDLTextureRAII> texture);

namespace details
{

/**
 * Loads a texture from the specified file path.
 *
 * @param renderer The SDL renderer used to load the texture.
 * @param filePath The file path of the texture to be loaded.
 * @return A shared pointer to the loaded texture.
 */
std::shared_ptr<SDLTextureRAII> LoadTexture(SDL_Renderer* renderer, const std::filesystem::path& imagePath);

/**
 * @brief Loads a texture with streaming access.
 *
 * This function loads a texture from the specified file path using SDL_Renderer with streaming access.
 *
 * @param renderer The SDL_Renderer pointer.
 * @param filePath The file path of the texture to load.
 * @return A shared pointer to the loaded texture.
 */
std::shared_ptr<SDLTextureRAII> LoadTextureWithStreamingAccess(
    SDL_Renderer* renderer, const std::filesystem::path& imagePath);

} // namespace details
