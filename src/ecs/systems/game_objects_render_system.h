#pragma once
#include <SDL.h>
#include <ecs/components/game_components.h>
#include <entt/entt.hpp>
#include <utils/coordinates_transformer.h>
#include <utils/sdl_colors.h>

class GameObjectsRenderSystem
{
    entt::registry& registry;
    SDL_Renderer* renderer;
    GameState& gameState;
    CoordinatesTransformer coordinatesTransformer;
public:
    GameObjectsRenderSystem(entt::registry& registry, SDL_Renderer* renderer);
    void Render();
private: // Render game objects methods.
    void RenderBackground();
    void RenderTiles();
    void RenderPlayerWeaponDirection();
private: // Render helpers. TODO: check if items can be removed.
    void RenderSquare(const glm::vec2& sdlPos, const glm::vec2& sdlSize, ColorName color, float angle);
    void RenderSquare(std::shared_ptr<Box2dObjectRAII> body, const glm::vec2& sdlSize, ColorName color);
    void RenderTiledSquare(std::shared_ptr<Box2dObjectRAII> body, const RenderingInfo& tileInfo);
    SDL_Rect GetRectWithCameraTransform(const glm::vec2& sdlPos, const glm::vec2& sdlSize);
};
