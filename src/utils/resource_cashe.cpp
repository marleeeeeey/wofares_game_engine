#include "resource_cashe.h"
#include <filesystem>
#include <my_common_cpp_utils/Logger.h>
#include <tuple>
#include <utils/texture_process.h>

ResourceCashe::ResourceCashe(SDL_Renderer* renderer) : renderer(renderer)
{}

std::shared_ptr<SDLTextureRAII> ResourceCashe::LoadTexture(std::filesystem::path& filePath, TextureAccess access)
{
    // Get absolute path to the file.
    std::filesystem::path absolutePath = std::filesystem::absolute(filePath);
    TextureKey key = std::make_tuple(absolutePath, access);

    // Return cached texture if it was already loaded.
    if (textures.contains(key))
        return textures[key];

    // Load the texture and cache it.
    std::shared_ptr<SDLTextureRAII> textureRAII;

    if (access == TextureAccess::Streaming)
        textureRAII = details::LoadTextureWithStreamingAccess(renderer, absolutePath);
    else if (access == TextureAccess::Static)
        textureRAII = details::LoadTexture(renderer, absolutePath);

    MY_LOG_FMT(info, "Loaded texture({}): {}", access, filePath.string());

    textures[key] = textureRAII;
    return textureRAII;
}
