#pragma once
#include <SDL.h>
#include <ecs/components/game_components.h>
#include <entt/entt.hpp>
#include <utils/coordinates_transformer.h>
#include <utils/resource_manager.h>
#include <utils/sdl_colors.h>

class GameObjectsRenderSystem
{
    entt::registry& registry;
    SDL_Renderer* renderer;
    GameOptions& gameState;
    CoordinatesTransformer coordinatesTransformer;
    ResourceManager& resourceManager;
public:
    GameObjectsRenderSystem(entt::registry& registry, SDL_Renderer* renderer, ResourceManager& resourceManager);
    void Render();
private: // Render game objects methods.
    void RenderBackground();
    void RenderTiles();
    void RenderAnimations();
    void RenderPlayerWeaponDirection();
private: // Render helpers. TODO: check if items can be removed.
    void RenderSquare(const glm::vec2& sdlPos, const glm::vec2& sdlSize, ColorName color, float angle);
    void RenderSquare(std::shared_ptr<Box2dObjectRAII> body, const glm::vec2& sdlSize, ColorName color);
    void RenderTiledSquare(
        std::shared_ptr<Box2dObjectRAII> body, const RenderingInfo& tileInfo,
        const SDL_RendererFlip& flip = SDL_FLIP_NONE);
    SDL_Rect GetRectWithCameraTransform(const glm::vec2& sdlPos, const glm::vec2& sdlSize);
};
