#pragma once
#include "SDL_render.h"
#include "ecs/components/game_components.h"
#include "utils/resource_cache.h"
#include "utils/sdl_RAII.h"
#include "utils/sdl_colors.h"
#include <filesystem>
#include <memory>
#include <unordered_map>

// High-level resource management. Get resources by friendly names in game like terminolgy.
// Every Get* method define specific resource type and return it by friendly name.
// Example of resource map file `resourceMapFilePath` content:
// {
//   "animations": {
//     "player_walk": "animations/player_walk.json"
//   },
//   "sounds": {
//     "background_music": "path/to/sounds/background_music.ogg",
//     "explosion": "path/to/sounds/explosion.wav"
//   },
//   "textures": {
//     "player_texture": "path/to/textures/player.png",
//     "enemy_texture": "path/to/textures/enemy.png"
//   },
//   "maps": {
//     "level1": "path/to/maps/level1.json",
//     "level2": "path/to/maps/level2.json"
//   }
// }
class ResourceManager
{
    details::ResourceCache resourceCashe;
    std::filesystem::path assetsDirectory; // Absolute path to the directory with assets.

    using FriendlyName = std::string;
    std::unordered_map<FriendlyName, AnimationInfo> animations;
    std::unordered_map<FriendlyName, std::filesystem::path> tiledLevels;
    std::unordered_map<FriendlyName, std::filesystem::path> musicPaths;
    std::unordered_map<std::string, std::vector<std::filesystem::path>> soundEffectPaths;
public:
    ResourceManager(const std::filesystem::path& assetsDirectory, SDL_Renderer* renderer);
public: // ************************* Animations *************************
    AnimationInfo GetAnimation(const std::string& name);
private:
    AnimationInfo ReadAsepriteAnimation(const std::filesystem::path& asepriteAnimationJsonPath);
public: // ************************* Tiled levels *************************
    std::filesystem::path GetTiledLevel(const std::string& name);
public: // ************************* Textures *************************
    std::shared_ptr<SDLTextureRAII> GetColoredPixelTexture(ColorName color);
    std::shared_ptr<SDLTextureRAII> GetTexture(const std::filesystem::path& path);
    std::shared_ptr<SDLSurfaceRAII> GetSurface(const std::filesystem::path& path);
public: // ************************* Sounds *************************
    std::shared_ptr<MusicRAII> GetMusic(const std::string& name);
    std::shared_ptr<SoundEffectRAII> GetSoundEffect(const std::string& name);
};