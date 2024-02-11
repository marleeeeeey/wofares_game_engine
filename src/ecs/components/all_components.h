#pragma once
#include <box2d/box2d.h>
#include <glm/glm.hpp>
#include <memory>
#include <utils/box2d_RAII.h>
#include <utils/sdl_RAII.h>

struct SizeComponent
{
    // Must be installed manually. Because this components is not calculated from Box2D body.
    glm::vec2 value = {0, 0};
};

struct PlayerNumber
{
    size_t value = 0;
};

struct GameState
{
    bool quit{false}; // Flag to control game loop exit
    glm::vec2 windowSize{800, 600};
    unsigned fps{60};
    float cameraScale{1.0f};
    glm::vec2 cameraCenter{};
    bool isSceneCaptured{false};
    std::shared_ptr<b2World> physicsWorld;
    bool reloadMap{false};
    float dynamicBodyProbability{0.3f};
    size_t miniTileResolution{4};
    int32 velocityIterations{6};
    int32 positionIterations{2};
    bool preventCreationInvisibleTiles{false};
    // Gap between physical and visual objects. Used to prevent dragging of physical objects.
    float gapBetweenPhysicalAndVisual{0.2f};
};

struct TileInfo
{
    std::shared_ptr<Texture> texture; // Pointer to the texture.
    SDL_Rect srcRect; // Rectangle in the texture corresponding to the tile.
};

struct PhysicalBody
{
    std::shared_ptr<Box2dObjectRAII> value;
};