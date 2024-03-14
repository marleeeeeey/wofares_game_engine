#pragma once
#include <SDL.h>
#include <ecs/components/game_components.h>
#include <entt/entt.hpp>
#include <utils/RAII/box2d_RAII.h>
#include <utils/coordinates_transformer.h>
#include <utils/game_options.h>
#include <utils/resources/resource_manager.h>
#include <utils/sdl_colors.h>

class PrimitivesRenderer
{
    entt::registry& registry;
    SDL_Renderer* renderer;
    GameOptions& gameState;
    CoordinatesTransformer coordinatesTransformer;
    ResourceManager& resourceManager;
public:
    PrimitivesRenderer(entt::registry& registry, SDL_Renderer* renderer, ResourceManager& resourceManager);
public:
    void RenderSquare(const glm::vec2& sdlPos, const glm::vec2& sdlSize, ColorName color, float angle);
    void RenderSquare(std::shared_ptr<Box2dObjectRAII> body, const glm::vec2& sdlSize, ColorName color);
    void RenderTiledSquare(
        const glm::vec2& centerSdlPos, const float angle, const RenderingInfo& tileInfo,
        const SDL_RendererFlip& flip = SDL_FLIP_NONE);
    SDL_Rect GetRectWithCameraTransform(const glm::vec2& sdlPos, const glm::vec2& sdlSize);
    void RenderAnimation(const AnimationInfo& animationInfo, glm::vec2 physicsBodyCenterWorld, float angle);
    void RenderBackground(const BackgroundInfo& backgroundInfo);
};