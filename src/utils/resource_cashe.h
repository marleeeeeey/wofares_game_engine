#pragma once
#include "SDL_render.h"
#include <memory>
#include <unordered_map>
#include <utils/sdl_RAII.h>

// Reponsible for low-level resource management like loading textures and sounds.
class ResourceCashe
{
    SDL_Renderer* renderer;

    // Map absolute file paths to the textures.
    std::unordered_map<std::string, std::shared_ptr<SDLTextureRAII>> textures;
public:
    ResourceCashe(SDL_Renderer* renderer);
    std::shared_ptr<SDLTextureRAII> LoadTexture(const std::string& filePath);
};