#pragma once
#include "SDL_render.h"
#include <glm/glm.hpp>
#include <string>

struct Position
{
    glm::vec2 value;
};

struct Velocity
{
    glm::vec2 value;
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
};

struct TileInfo
{
    int tileId;
    // TODO: Additional tile properties can be added here.
};

struct Renderable
{
    SDL_Texture* texture; // Pointer to the texture.
    SDL_Rect srcRect; // Rectangle in the texture corresponding to the tile.
};
