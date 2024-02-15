#pragma once
#include <SDL.h>
#include <ecs/components/all_components.h>
#include <entt/entt.hpp>
#include <utils/sdl_colors.h>

class GameObjectsRenderer
{
    entt::registry& registry;
    SDL_Renderer* renderer;
    GameState& gameState;
public:
    GameObjectsRenderer(entt::registry& registry, SDL_Renderer* renderer);
private: // Render game objects methods.
    void RenderTiles();
    void RenderPlayers();
    void RenderGranades();
private: // Render helpers.
    void RenderSquare(std::shared_ptr<Box2dObjectRAII> body, const glm::vec2& sdlSize, ColorName color);
    void RenderTiledSquare(std::shared_ptr<Box2dObjectRAII> body, const glm::vec2& sdlSize, const TileInfo& tileInfo);
};
