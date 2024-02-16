#pragma once
#include <SDL.h>
#include <ecs/components/game_components.h>
#include <ecs/systems/details/coordinates_transformer.h>
#include <entt/entt.hpp>
#include <utils/sdl_colors.h>

class GameObjectsRenderer
{
    entt::registry& registry;
    SDL_Renderer* renderer;
    GameState& gameState;
    CoordinatesTransformer coordinatesTransformer;
public:
    GameObjectsRenderer(entt::registry& registry, SDL_Renderer* renderer);
private: // Render game objects methods.
    void RenderTiles();
    void RenderPlayers();
    void RenderGranades();
    void RenderBridges();
private: // Render helpers.
    void RenderSquare(const glm::vec2& sdlPos, const glm::vec2& sdlSize, ColorName color, float angle);
    void RenderSquare(std::shared_ptr<Box2dObjectRAII> body, const glm::vec2& sdlSize, ColorName color);
    void RenderTiledSquare(std::shared_ptr<Box2dObjectRAII> body, const glm::vec2& sdlSize, const TileInfo& tileInfo);
    SDL_Rect GetRectWithCameraTransform(const glm::vec2& sdlPos, const glm::vec2& sdlSize);
};
