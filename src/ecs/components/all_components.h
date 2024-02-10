#pragma once
#include <box2d/box2d.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <utils/box2d_RAII.h>
#include <utils/sdl_RAII.h>

struct Position
{
    glm::vec2 value;
};

struct Angle
{
    float value; // In radians.
};

struct SizeComponent
{
    glm::vec2 value;
};

struct PlayerNumber
{
    size_t value;
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