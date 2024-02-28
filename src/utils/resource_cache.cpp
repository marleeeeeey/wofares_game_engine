#include "resource_cache.h"
#include <filesystem>
#include <my_common_cpp_utils/Logger.h>
#include <utils/texture_process.h>

namespace details
{

ResourceCache::ResourceCache(SDL_Renderer* renderer) : renderer(renderer)
{}

std::shared_ptr<SDLTextureRAII> ResourceCache::LoadTexture(const std::filesystem::path& filePath)
{
    // Get absolute path to the file.
    std::filesystem::path absolutePath = std::filesystem::absolute(filePath);

    // Return cached texture if it was already loaded.
    if (textures.contains(absolutePath))
        return textures[absolutePath];

    // Load the texture and cache it.
    std::shared_ptr<SDLTextureRAII> textureRAII;
    textureRAII = details::LoadTexture(renderer, absolutePath);
    MY_LOG_FMT(info, "Loaded texture: {}", filePath.string());
    textures[absolutePath] = textureRAII;
    return textureRAII;
}

std::shared_ptr<MusicRAII> ResourceCache::LoadMusic(const std::filesystem::path& filePath)
{
    // Get absolute path to the file.
    std::filesystem::path absolutePath = std::filesystem::absolute(filePath);

    // Return cached music if it was already loaded.
    if (musics.contains(absolutePath))
        return musics[absolutePath];

    // Load the music and cache it.
    std::shared_ptr<MusicRAII> musicRAII = std::make_shared<MusicRAII>(absolutePath.string());
    musics[absolutePath] = musicRAII;
    return musicRAII;
};

std::shared_ptr<SoundEffectRAII> ResourceCache::LoadSoundEffect(const std::filesystem::path& filePath)
{
    // Get absolute path to the file.
    std::filesystem::path absolutePath = std::filesystem::absolute(filePath);

    // Return cached sound effect if it was already loaded.
    if (soundEffects.contains(absolutePath))
        return soundEffects[absolutePath];

    // Load the sound effect and cache it.
    std::shared_ptr<SoundEffectRAII> soundEffectRAII = std::make_shared<SoundEffectRAII>(absolutePath.string());
    soundEffects[absolutePath] = soundEffectRAII;
    return soundEffectRAII;
};

std::shared_ptr<SDLSurfaceRAII> ResourceCache::LoadSurface(const std::filesystem::path& filePath)
{
    // Get absolute path to the file.
    std::filesystem::path absolutePath = std::filesystem::absolute(filePath);

    // Return cached surface if it was already loaded.
    if (surfaces.contains(absolutePath))
        return surfaces[absolutePath];

    // Load the surface and cache it.
    std::shared_ptr<SDLSurfaceRAII> surfaceRAII = details::LoadSurfaceWithStreamingAccess(renderer, absolutePath);
    surfaces[absolutePath] = surfaceRAII;
    return surfaceRAII;
};

} // namespace details