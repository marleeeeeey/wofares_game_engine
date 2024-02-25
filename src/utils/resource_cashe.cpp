#include "resource_cashe.h"
#include <filesystem>
#include <utils/texture_process.h>

ResourceCashe::ResourceCashe(SDL_Renderer* renderer) : renderer(renderer)
{}

std::shared_ptr<SDLTextureRAII> ResourceCashe::LoadTexture(const std::string& filePath)
{
    // Get absolute path to the file.
    std::filesystem::path absolutePath = std::filesystem::absolute(filePath);

    // Return cached texture if it was already loaded.
    if (textures.contains(absolutePath.string()))
        return textures[absolutePath.string()];

    // Load the texture and cache it.
    auto textureRAII = ::LoadTexture(renderer, absolutePath);
    textures[absolutePath.string()] = textureRAII;
    return textureRAII;
}
