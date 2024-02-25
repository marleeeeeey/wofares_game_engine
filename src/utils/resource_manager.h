#pragma once
#include "ecs/components/game_components.h"
#include "utils/resource_cashe.h"
#include <filesystem>
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
    ResourceCashe& resourceCashe;
    std::filesystem::path resourceMapJson; // Absolute path to the resource map file.
    std::filesystem::path assetsDirectory; // Absolute path to the directory with assets.

    using FriendlyName = std::string;
    std::unordered_map<FriendlyName, AnimationInfo> animations;
    std::unordered_map<FriendlyName, std::filesystem::path> tiledLevels;
public:
    ResourceManager(ResourceCashe& resourceCashe, const std::filesystem::path& resourceMapFilePath);
public: // ************************* Animations *************************
    AnimationInfo GetAnimation(const std::string& name);
private:
    AnimationInfo ReadAsepriteAnimation(const std::filesystem::path& asepriteAnimationJsonPath);
public: // ************************* Tiled levels *************************
    std::filesystem::path GetTiledLevel(const std::string& name);
};