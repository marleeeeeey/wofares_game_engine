#pragma once
#include <box2d/box2d.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <utils/box2d_RAII.h>
#include <utils/sdl_RAII.h>

struct Position
{
    glm::vec2 value = {0, 0};
};

struct Angle
{
    float value = 0; // In radians.
};

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
    float gravity = 1000.f;
    float cameraScale{1.0f};
    std::string debugMsg;
    std::string debugMsg2;
    glm::vec2 cameraCenter{};
    bool isSceneCaptured{false};
    std::shared_ptr<b2World> physicsWorld;
};

struct TileInfo
{
    std::shared_ptr<Texture> texture; // Pointer to the texture.
    SDL_Rect srcRect; // Rectangle in the texture corresponding to the tile.
};

struct PhysicalBody
{
    std::shared_ptr<Box2dObjectRAII> body;
};