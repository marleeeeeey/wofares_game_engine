#include "resource_manager.h"
#include "my_common_cpp_utils/Logger.h"
#include "my_common_cpp_utils/MathUtils.h"
#include "utils/resource_cache.h"
#include <cstddef>
#include <filesystem>
#include <fstream>

ResourceManager::ResourceManager(const std::filesystem::path& resourceMapFilePath, SDL_Renderer* renderer)
  : resourceCashe(renderer)
{
    resourceMapJson = std::filesystem::absolute(resourceMapFilePath);
    if (!std::filesystem::exists(resourceMapJson))
        throw std::runtime_error(MY_FMT("Resource map file does not found: {}", resourceMapJson.string()));
    assetsDirectory = resourceMapJson.parent_path();

    std::ifstream resourceMapJsonFile(resourceMapJson);
    nlohmann::json resourceMapJsonData;
    resourceMapJsonFile >> resourceMapJsonData;

    // Load animations.
    for (const auto& animationPair : resourceMapJsonData["animations"].items())
    {
        const std::string& animationName = animationPair.key();
        const std::string& animationPath = animationPair.value().get<std::string>();
        std::filesystem::path asepriteAnimationJsonPath = assetsDirectory / animationPath;
        animations[animationName] = ReadAsepriteAnimation(asepriteAnimationJsonPath);
    }

    // Load tiled level names.
    for (const auto& tiledLevelPair : resourceMapJsonData["maps"].items())
    {
        const std::string& tiledLevelName = tiledLevelPair.key();
        const std::string& tiledLevelPath = tiledLevelPair.value().get<std::string>();
        std::filesystem::path tiledLevelJsonPath = assetsDirectory / tiledLevelPath;
        if (!std::filesystem::exists(tiledLevelJsonPath))
            throw std::runtime_error(MY_FMT("Tiled level file does not found: {}", tiledLevelJsonPath.string()));
        tiledLevels[tiledLevelName] = tiledLevelJsonPath;
    }

    // Load sound effects.
    for (const auto& soundEffectPair : resourceMapJsonData["sound_effects"].items())
    {
        const std::string& soundEffectName = soundEffectPair.key();
        const auto& soundEffectPathsJson = soundEffectPair.value();

        if (!soundEffectPathsJson.is_array())
            throw std::runtime_error(MY_FMT("Sound effect paths for '{}' should be an array", soundEffectName));

        std::vector<std::filesystem::path> paths;

        // Load sound effect paths.
        for (const auto& pathJson : soundEffectPathsJson)
        {
            const std::string& soundEffectPath = pathJson.get<std::string>();
            std::filesystem::path soundEffectAbsolutePath = assetsDirectory / soundEffectPath;
            paths.push_back(soundEffectAbsolutePath);
        }

        soundEffectPaths[soundEffectName] = paths;
    }

    // Load music.
    for (const auto& musicPair : resourceMapJsonData["music"].items())
    {
        const std::string& musicName = musicPair.key();
        const std::string& musicPath = musicPair.value().get<std::string>();
        std::filesystem::path musicAbsolutePath = assetsDirectory / musicPath;
        musicPaths[musicName] = musicAbsolutePath;
    }

    MY_LOG_FMT(
        info, "Game found {} animation(s), {} level(s), {} music(s), {} sound(s).", animations.size(),
        tiledLevels.size(), musicPaths.size(), soundEffectPaths.size());
}

AnimationInfo ResourceManager::GetAnimation(const std::string& name)
{
    if (!animations.contains(name))
        throw std::runtime_error(MY_FMT("Animation with name '{}' does not found", name));
    return animations[name];
}

/**
 * Load aseprite animation from a json file. Example of the json file:
 * {
 *   "frames": {
 *     "m_walk 0.png": {
 *       "frame": { "x": 0, "y": 0, "w": 9, "h": 19 },
 *       ...
 *       "duration": 100
 *     },
 *     ...
 *   },
 *   "meta": {
 *     ...
 *     "image": "SPRITE-SHEET.png",
 *     ...
 *   }
 * }
 */
AnimationInfo ResourceManager::ReadAsepriteAnimation(const std::filesystem::path& asepriteAnimationJsonPath)
{
    // Read aseprite animation json file.
    std::ifstream asepriteAnimationJsonFile(asepriteAnimationJsonPath);
    if (!asepriteAnimationJsonFile.is_open())
    {
        throw std::runtime_error(
            MY_FMT("Failed to open aseprite animation file: {}", asepriteAnimationJsonPath.string()));
    }
    nlohmann::json asepriteJsonData;
    asepriteAnimationJsonFile >> asepriteJsonData;

    // Load texture.
    const std::string imagePath = asepriteJsonData["meta"]["image"].get<std::string>();
    auto animationTexturePath = asepriteAnimationJsonPath.parent_path() / imagePath;
    auto textureRAII = resourceCashe.LoadTexture(animationTexturePath);

    // Create animation.
    AnimationInfo animation;
    for (const auto& framePair : asepriteJsonData["frames"].items())
    {
        // Read frame info.
        const auto& frame = framePair.value()["frame"];
        int x = frame["x"];
        int y = frame["y"];
        int w = frame["w"];
        int h = frame["h"];
        int duration = framePair.value()["duration"];

        // Create animation frame.
        AnimationFrame animationFrame;
        animationFrame.renderingInfo.texturePtr = textureRAII;
        animationFrame.renderingInfo.textureRect = {x, y, w, h};
        animationFrame.renderingInfo.sdlSize = {w, h}; // TODO: obsolete. Remove later.
        animationFrame.duration = static_cast<float>(duration) / 1000.0f; // Convert to seconds.

        // Add frame to the animation.
        animation.frames.push_back(std::move(animationFrame));
    }
    return animation;
};

std::filesystem::path ResourceManager::GetTiledLevel(const std::string& name)
{
    if (!tiledLevels.contains(name))
        throw std::runtime_error(MY_FMT("Tiled level with name '{}' does not found", name));
    return tiledLevels[name];
}

std::shared_ptr<SDLTextureRAII> ResourceManager::GetTexture(const std::filesystem::path& path)
{
    return resourceCashe.LoadTexture(path);
};

std::shared_ptr<MusicRAII> ResourceManager::GetMusic(const std::string& name)
{
    if (!musicPaths.contains(name))
        throw std::runtime_error(MY_FMT("Music with name '{}' does not found", name));
    return resourceCashe.LoadMusic(musicPaths[name]);
};

std::shared_ptr<SoundEffectRAII> ResourceManager::GetSoundEffect(const std::string& name)
{
    if (!soundEffectPaths.contains(name))
        throw std::runtime_error(MY_FMT("Sound effect with name '{}' does not found", name));

    // Get random sound effect from the list.
    const auto& sounds = soundEffectPaths[name];
    auto number = utils::random<size_t>(0, soundEffectPaths[name].size() - 1);
    return resourceCashe.LoadSoundEffect(sounds[number]);
};

std::shared_ptr<SDLSurfaceRAII> ResourceManager::GetSurface(const std::filesystem::path& path)
{
    return resourceCashe.LoadSurface(path);
};

std::shared_ptr<SDLTextureRAII> ResourceManager::GetColoredPixelTexture(ColorName color)
{
    return resourceCashe.GetColoredPixelTexture(color);
};