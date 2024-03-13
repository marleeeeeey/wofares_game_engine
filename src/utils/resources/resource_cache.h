#pragma once
#include "SDL_render.h"
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <utils/RAII/sdl_RAII.h>
#include <utils/RAII/sdl_audio_RAII.h>
#include <utils/sdl_colors.h>

namespace details
{

// Reponsible for low-level resource management like loading textures and sounds.
class ResourceCache
{
public:
    explicit ResourceCache(SDL_Renderer* renderer);

    std::shared_ptr<SDLTextureRAII> GetColoredPixelTexture(const ColorName& color);
    std::shared_ptr<SDLTextureRAII> LoadTexture(const std::filesystem::path& filePath);
    std::shared_ptr<SDLSurfaceRAII> LoadSurface(const std::filesystem::path& filePath);
    std::shared_ptr<MusicRAII> LoadMusic(const std::filesystem::path& filePath);
    std::shared_ptr<SoundEffectRAII> LoadSoundEffect(const std::filesystem::path& filePath);
private:
    SDL_Renderer* renderer;

    // Map absolute file paths to the textures/sounds.
    std::unordered_map<ColorName, std::shared_ptr<SDLTextureRAII>> coloredTextures;
    std::unordered_map<std::filesystem::path, std::shared_ptr<SDLTextureRAII>> textures;
    std::unordered_map<std::filesystem::path, std::shared_ptr<SDLSurfaceRAII>> surfaces;
    std::unordered_map<std::filesystem::path, std::shared_ptr<MusicRAII>> musics;
    std::unordered_map<std::filesystem::path, std::shared_ptr<SoundEffectRAII>> soundEffects;
};
} // namespace details