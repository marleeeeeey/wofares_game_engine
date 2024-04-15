#pragma once
#include <box2d/box2d.h>
#include <glm/glm.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <utils/sdl/sdl_RAII.h>

struct LevelPhysicsBounds
{
    b2Vec2 min = b2Vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    b2Vec2 max = b2Vec2(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());
};

struct BackgroundInfo
{
    std::shared_ptr<SDLTextureRAII> texture{}; // Pointer to the texture.
    float textureScale{1.0f}; // Scale of the texture. 1.0f means that the texture is not scaled.
    float speedFactor{0.1f}; // Speed factor of the background. 1.0f means that the background moves with the camera.
};

struct LevelOptions
{
    BackgroundInfo backgroundInfo;
    LevelPhysicsBounds levelBox2dBounds;
    b2Vec2 bufferZone{3.0f, 3.0f};
};

struct WindowOptions
{
    glm::vec2 windowSize{1600, 900}; // TODO4: support for json serialization.
    float cameraScale{1.0f};
    glm::vec2 cameraCenterSdl{};
    glm::vec2 lastMousePosInWindow{};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(WindowOptions, cameraScale)
};

struct ControlOptions
{
    bool quit{false}; // Flag to control game loop exit
    bool reloadMap{false};
    bool isSceneCaptured{false};
};

struct DebugInfo
{
    float spacePressedDuration{0.0f};
    float spacePressedDurationOnUpEvent{0.0f};
};

struct GameOptions
{
    std::shared_ptr<b2World> physicsWorld;
    LevelOptions levelOptions;
    WindowOptions windowOptions;
    ControlOptions controlOptions;
    DebugInfo debugInfo;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(GameOptions, windowOptions)
};
