#pragma once
#include <box2d/box2d.h>
#include <glm/glm.hpp>
#include <memory>
#include <utils/box2d_RAII.h>
#include <utils/sdl_RAII.h>

// ************************************ COMPONENTS ************************************

struct SdlSizeComponent
{
    // Must be installed manually. Because this components is not calculated from Box2D body.
    glm::vec2 value = {0, 0};
};

struct PlayerNumber
{
    size_t value = 0;
};

struct PlayerDirection
{
    // {1, 0} - look to the right,
    // {-1, 0} - look to the left,
    // {0, 1} - look down,
    // {0, -1} - look up
    glm::vec2 direction = {1, 0};
};

struct TileInfo
{
    std::shared_ptr<SDLTextureRAII> texture; // Pointer to the texture.
    SDL_Rect srcRect; // Rectangle in the texture corresponding to the tile.
};

struct PhysicalBody
{
    std::shared_ptr<Box2dObjectRAII> value;
};

struct Granade
{
    float timeToExplode = 3.0f; // Time in seconds before the grenade explodes
};

// ********************************* SINGLETON COMPONENT *********************************

struct LevelPhysicsBounds
{
    b2Vec2 min = b2Vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    b2Vec2 max = b2Vec2(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());
};

struct LevelOptions
{
    LevelPhysicsBounds levelBox2dBounds;
    b2Vec2 bufferZone{10.0f, 10.0f};
    float dynamicBodyProbability{0.3f};
    size_t miniTileResolution{4};
    bool preventCreationInvisibleTiles{false};
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

struct RenderingOptions
{
    unsigned fps{60};
    glm::vec2 windowSize{800, 600};
    float cameraScale{1.0f};
    glm::vec2 cameraCenter{};
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

struct GameState
{
    std::shared_ptr<b2World> physicsWorld;
    LevelOptions levelOptions;
    PhysicsOptions physicsOptions;
    RenderingOptions renderingOptions;
    ControlOptions controlOptions;
    DebugInfo debugInfo;
};
