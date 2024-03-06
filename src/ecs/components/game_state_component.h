#pragma once
#include <box2d/box2d.h>
#include <glm/glm.hpp>
#include <memory>
#include <utils/sdl_RAII.h>

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
    b2Vec2 bufferZone{10.0f, 10.0f};
    float dynamicBodyProbability{0.0f};
    size_t miniTileResolution{2};
    bool preventCreationInvisibleTiles{true}; // TODO: implement this feature.
    float colisionDisableProbability{0.7f};
};

struct PhysicsOptions
{
    int32 velocityIterations{6};
    int32 positionIterations{2};
    // Gap between physical and visual objects. Used to prevent dragging of physical objects.
    // Also affects the destructibility of stacks of tiles. The smaller the gap, the easier it is to destroy the stack.
    // The bigger the gap, the harder it is to destroy the stack => less random destruction.
    float gapBetweenPhysicalAndVisual{0.5f};
};

struct WindowOptions
{
    unsigned fps{60};
    glm::vec2 windowSize{800, 600};
    float cameraScale{1.0f};
    glm::vec2 cameraCenterSdl{};
    glm::vec2 lastMousePosInWindow{};
    float box2DtoSDL = 48.0f; // 1 meter in Box2D is XX pixels in SDL.
    bool showGrid{false};
};

struct ControlOptions
{
    bool quit{false}; // Flag to control game loop exit
    bool reloadMap{false};
    bool isSceneCaptured{false};
};

struct SoundOptions
{
    float randomSoundEventInterval{10}; // Interval between random sound events in seconds.
    float nextSoundEventTime{0.0f}; // Time of the next random sound event.
};

struct DebugInfo
{
    float spacePressedDuration{0.0f};
    float spacePressedDurationOnUpEvent{0.0f};
};

struct GameState
{
    std::shared_ptr<b2World> physicsWorld;
    LevelOptions levelOptions;
    PhysicsOptions physicsOptions;
    WindowOptions windowOptions;
    ControlOptions controlOptions;
    SoundOptions soundOptions;
    DebugInfo debugInfo;
};
