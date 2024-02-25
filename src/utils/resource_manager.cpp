#include "resource_manager.h"
#include "my_common_cpp_utils/Logger.h"
#include <filesystem>
#include <fstream>

ResourceManager::ResourceManager(ResourceCashe& resourceCashe, const std::filesystem::path& resourceMapFilePath)
  : resourceCashe(resourceCashe), resourceMapJson()
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
}

AnimationInfo ResourceManager::GetAnimation(const std::string& name)
{
    return animations.at(name);
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
 *     IMPORTANT: Path to the sprite sheet should be full! Need to be changed. By default it is relative.
 *     "image": "FULL\PATH\TO\SPRITE-SHEET.png",
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
    auto imageAbsolutePath = assetsDirectory / "images" / imagePath;
    auto textureRAII = resourceCashe.LoadTexture(imageAbsolutePath.string());

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
        animationFrame.duration = static_cast<float>(duration) / 1000.0f; // Convert to seconds.

        // Add frame to the animation.
        animation.frames.push_back(std::move(animationFrame));
    }
    return animation;
};