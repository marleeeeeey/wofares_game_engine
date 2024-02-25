#pragma once
#include "SDL_render.h"
#include <filesystem>
#include <map>
#include <memory>
#include <utils/sdl_RAII.h>

namespace details
{
// Reponsible for low-level resource management like loading textures and sounds.
class ResourceCashe
{
public:
    explicit ResourceCashe(SDL_Renderer* renderer);

    enum class TextureAccess
    {
        Streaming,
        Static
    };
    std::shared_ptr<SDLTextureRAII> LoadTexture(
        const std::filesystem::path& filePath, TextureAccess access = TextureAccess::Static);
private:
    SDL_Renderer* renderer;

    // Map absolute file paths to the textures.
    using TextureKey = std::tuple<std::filesystem::path, TextureAccess>;
    std::map<TextureKey, std::shared_ptr<SDLTextureRAII>> textures;
};
} // namespace details