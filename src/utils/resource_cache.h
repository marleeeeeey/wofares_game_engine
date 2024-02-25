#pragma once
#include "SDL_render.h"
#include <filesystem>
#include <map>
#include <memory>
#include <unordered_map>
#include <utils/sdl_RAII.h>
#include <utils/sdl_audio_RAII.h>

namespace details
{
// Reponsible for low-level resource management like loading textures and sounds.
class ResourceCache
{
public:
    explicit ResourceCache(SDL_Renderer* renderer);

    enum class TextureAccess
    {
        Streaming,
        Static
    };
    std::shared_ptr<SDLTextureRAII> LoadTexture(
        const std::filesystem::path& filePath, TextureAccess access = TextureAccess::Static);
    std::shared_ptr<MusicRAII> LoadMusic(const std::filesystem::path& filePath);
    std::shared_ptr<SoundEffectRAII> LoadSoundEffect(const std::filesystem::path& filePath);
private:
    SDL_Renderer* renderer;

    // Map absolute file paths to the textures/sounds.
    using TextureKey = std::tuple<std::filesystem::path, TextureAccess>;
    std::map<TextureKey, std::shared_ptr<SDLTextureRAII>> textures;
    std::unordered_map<std::filesystem::path, std::shared_ptr<MusicRAII>> musics;
    std::unordered_map<std::filesystem::path, std::shared_ptr<SoundEffectRAII>> soundEffects;
};
} // namespace details