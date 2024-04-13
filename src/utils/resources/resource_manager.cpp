#include "resource_manager.h"
#include <SDL_rect.h>
#include <cstddef>
#include <filesystem>
#include <glob/glob.hpp>
#include <my_cpp_utils/config.h>
#include <my_cpp_utils/dict_utils.h>
#include <my_cpp_utils/json_utils.h>
#include <my_cpp_utils/math_utils.h>
#include <my_cpp_utils/string_utils.h>
#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>
#include <utils/logger.h>
#include <utils/resources/aseprite_data.h>
#include <utils/resources/resource_cache.h>
#include <utils/sdl_texture_process.h>

ResourceManager::ResourceManager(SDL_Renderer* renderer, const nlohmann::json& assetsSettingsJson)
  : resourceCashe(renderer)
{
    // Load animations.
    for (const auto& animationPair : assetsSettingsJson["animations"].items())
    {
        const std::string& animationName = animationPair.key();
        auto animationPath = animationPair.value().get<std::filesystem::path>();
        animations[animationName] = ReadAsepriteAnimation(animationPath);
    }

    // Load tiled level names.
    for (const auto& tiledLevelPair : assetsSettingsJson["maps"].items())
    {
        LevelInfo levelInfo = tiledLevelPair.value().get<LevelInfo>();
        if (!std::filesystem::exists(levelInfo.tiledMapPath))
            throw std::runtime_error(MY_FMT("Tiled level file does not found: {}", levelInfo.tiledMapPath.string()));
        tiledLevels[levelInfo.name] = levelInfo;
    }

    // Load sound effects.
    for (const auto& soundEffectPair : assetsSettingsJson["sound_effects"].items())
    {
        const std::string& soundEffectName = soundEffectPair.key();
        const auto& soundEffectGlobsJson = soundEffectPair.value();

        if (!soundEffectGlobsJson.is_array())
            throw std::runtime_error(MY_FMT("Sound effect paths for '{}' should be an array", soundEffectName));

        std::vector<std::filesystem::path> paths;

        // Load sound effect paths.
        for (const auto& soundEffectGlobPath : soundEffectGlobsJson)
        {
            for (auto& soundEffectPath : glob::glob(soundEffectGlobPath.get<std::string>()))
            {
                paths.push_back(soundEffectPath);
            }
        }

        soundEffectPaths[soundEffectName] = paths;
    }

    // Load music.
    for (const auto& musicPair : assetsSettingsJson["music"].items())
    {
        const std::string& musicName = musicPair.key();
        const auto musicPath = musicPair.value().get<std::filesystem::path>();
        musicPaths[musicName] = musicPath;
    }

    MY_LOG(
        info, "Game found {} animation(s), {} level(s), {} music(s), {} sound effect(s).", animations.size(),
        tiledLevels.size(), musicPaths.size(), soundEffectPaths.size());
}

Animation ResourceManager::GetAnimation(const std::string& animationName)
{
    if (!animations.contains(animationName))
        throw std::runtime_error(MY_FMT("Animation with name '{}' does not found", animationName));

    if (animations[animationName].size() != 1)
        throw std::runtime_error(MY_FMT("Animation with name '{}' has more than one tag", animationName));

    // Get first animation tag.
    return animations[animationName].begin()->second;
}

Animation ResourceManager::GetAnimation(const std::string& animationName, const std::string& tagName, TagProps tagProps)
{
    if (tagProps == TagProps::ExactMatch)
        return GetAnimationExactMatch(animationName, tagName);

    if (tagProps == TagProps::RandomByRegex)
        return GetAnimationByRegexRandomly(animationName, tagName);

    throw std::runtime_error(MY_FMT("Unknown TagProps: {}", static_cast<int>(tagProps)));
}

Animation ResourceManager::GetAnimationExactMatch(const std::string& animationName, const std::string& tagName)
{
    if (!animations.contains(animationName))
        throw std::runtime_error(MY_FMT("Animation with name '{}' does not found", animationName));

    if (!animations[animationName].contains(tagName))
        throw std::runtime_error(MY_FMT("Animation tag with name '{}' does not found in {}", tagName, animationName));

    return animations[animationName][tagName];
}

Animation ResourceManager::GetAnimationByRegexRandomly(
    const std::string& animationName, const std::string& regexTagName)
{
    if (!animations.contains(animationName))
        throw std::runtime_error(MY_FMT("Animation with name '{}' does not found", animationName));

    std::vector<std::string> foundTags;
    for (const auto& [tag, _] : animations[animationName])
    {
        if (std::regex_match(tag, std::regex(regexTagName)))
            foundTags.push_back(tag);
    }

    if (foundTags.empty())
        throw std::runtime_error(
            MY_FMT("Animation tag with regex '{}' does not found in {}", regexTagName, animationName));

    auto randomTag = utils::Random<size_t>(0, foundTags.size() - 1);
    return animations[animationName][foundTags[randomTag]];
}

namespace
{

AnimationFrame GetAnimationFrameFromAsepriteFrame(
    const AsepriteData::Frame& asepriteFrame, std::shared_ptr<SDLTextureRAII> textureRAII)
{
    AnimationFrame animationFrame;
    animationFrame.renderingInfo.texturePtr = textureRAII;
    animationFrame.renderingInfo.textureRect = asepriteFrame.rectInTexture;
    animationFrame.renderingInfo.sizeWorld = {asepriteFrame.rectInTexture.w, asepriteFrame.rectInTexture.h};
    animationFrame.duration = asepriteFrame.duration_seconds;
    return animationFrame;
}

} // namespace

ResourceManager::TagToAnimationDict ResourceManager::ReadAsepriteAnimation(
    const std::filesystem::path& asepriteAnimationJsonPath)
{
    auto asepriteJsonData = utils::LoadJsonFromFile(asepriteAnimationJsonPath);

    AsepriteData asepriteData;
    try
    {
        asepriteData = LoadAsepriteData(asepriteJsonData);
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(MY_FMT(
            "[ReadAsepriteAnimation] Failed to load Aseprite data from '{}': {}", asepriteAnimationJsonPath.string(),
            e.what()));
    }

    // Load texture.
    auto animationTexturePath = asepriteAnimationJsonPath.parent_path() / asepriteData.texturePath;
    std::shared_ptr<SDLTextureRAII> textureRAII = resourceCashe.LoadTexture(animationTexturePath);
    // Surface with sreaming access is needed to get hitbox rect.
    std::shared_ptr<SDLSurfaceRAII> surfaceRAII = resourceCashe.LoadSurface(animationTexturePath);

    TagToAnimationDict tagToAnimationDict;

    if (!asepriteData.frameTags.empty())
    {
        std::optional<SDL_Rect> hitboxRect;
        if (asepriteData.frameTags.contains("Hitbox"))
        {
            const SDL_Rect& rectInSurface = asepriteData.frames[asepriteData.frameTags["Hitbox"].from].rectInTexture;
            hitboxRect = GetVisibleRectInSrcRectCoordinates(surfaceRAII->get(), rectInSurface);
            MY_LOG(
                debug, "Hitbox rect found: x={}, y={}, w={}, h={}", hitboxRect->x, hitboxRect->y, hitboxRect->w,
                hitboxRect->h);
        }

        for (const auto& [_, frameTag] : asepriteData.frameTags)
        {
            if (frameTag.name == "Hitbox")
                continue; // Skip hitbox tag (it's not an animation tag, it's a tag for hitbox frame).

            Animation animation;
            animation.hitboxRect = hitboxRect;
            for (size_t i = frameTag.from; i <= frameTag.to; ++i)
            {
                AnimationFrame animationFrame = GetAnimationFrameFromAsepriteFrame(asepriteData.frames[i], textureRAII);

                MY_LOG(
                    debug, "Frame {} has texture rect: x={}, y={}, w={}, h={}", i,
                    animationFrame.renderingInfo.textureRect.x, animationFrame.renderingInfo.textureRect.y,
                    animationFrame.renderingInfo.textureRect.w, animationFrame.renderingInfo.textureRect.h);

                animation.frames.push_back(std::move(animationFrame));
            }
            tagToAnimationDict[frameTag.name] = animation;
        }
    }
    else
    {
        // If there are no tags, then create one animation with all frames.
        Animation animation;
        for (size_t i = 0; i < asepriteData.frames.size(); ++i)
        {
            AnimationFrame animationFrame = GetAnimationFrameFromAsepriteFrame(asepriteData.frames[i], textureRAII);
            animation.frames.push_back(std::move(animationFrame));
        }
        tagToAnimationDict[""] = animation;
    }

    // Log names of loaded animations and tags.
    MY_LOG(
        info, "Loaded animation from '{}': {}", asepriteAnimationJsonPath.string(),
        utils::JoinStrings(utils::GetKeys(tagToAnimationDict), ", "));
    for (const auto& [tag, animation] : tagToAnimationDict)
    {
        MY_LOG(
            debug, "  Tag '{}' has {} frame(s), hitbox rect found: {}", tag, animation.frames.size(),
            animation.hitboxRect.has_value());
    }

    return tagToAnimationDict;
}

LevelInfo ResourceManager::GetTiledLevel(const std::string& name)
{
    if (!tiledLevels.contains(name))
        throw std::runtime_error(MY_FMT("Tiled level with name '{}' does not found", name));
    return tiledLevels[name];
}

std::shared_ptr<SDLTextureRAII> ResourceManager::GetTexture(const std::filesystem::path& path)
{
    return resourceCashe.LoadTexture(path);
}

std::shared_ptr<MusicRAII> ResourceManager::GetMusic(const std::string& name)
{
    if (!musicPaths.contains(name))
        throw std::runtime_error(MY_FMT("Music with name '{}' does not found", name));
    return resourceCashe.LoadMusic(musicPaths[name]);
}

std::shared_ptr<SoundEffectRAII> ResourceManager::GetSoundEffect(const std::string& name)
{
    if (!soundEffectPaths.contains(name))
        throw std::runtime_error(MY_FMT("Sound effect with name '{}' does not found", name));

    // Get random sound effect from the list.
    const auto& sounds = soundEffectPaths[name];
    auto number = utils::Random<size_t>(0, soundEffectPaths[name].size() - 1);
    return resourceCashe.LoadSoundEffect(sounds[number]);
}

std::shared_ptr<SDLSurfaceRAII> ResourceManager::GetSurface(const std::filesystem::path& path)
{
    return resourceCashe.LoadSurface(path);
}

std::shared_ptr<SDLTextureRAII> ResourceManager::GetColoredPixelTexture(ColorName color)
{
    return resourceCashe.GetColoredPixelTexture(color);
}