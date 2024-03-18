#pragma once
#include <SDL.h>
#include <ecs/components/animation_components.h>
#include <ecs/components/rendering_components.h>
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
    ResourceManager& resourceManager;
    GameOptions& gameState;
    CoordinatesTransformer coordinatesTransformer;
public:
    PrimitivesRenderer(entt::registry& registry, SDL_Renderer* renderer, ResourceManager& resourceManager);
public:
    void RenderSquare(const glm::vec2& posWorld, const glm::vec2& sizeWorld, ColorName color, float angle);
    void RenderSquare(std::shared_ptr<Box2dObjectRAII> body, const glm::vec2& sizeWorld, ColorName color);
    void RenderTiledSquare(
        const glm::vec2& centerWorld, const float angle, const RenderingComponent& tileInfo,
        const SDL_RendererFlip& flip = SDL_FLIP_NONE);
    SDL_Rect GetRectWithCameraTransform(const glm::vec2& posWorld, const glm::vec2& sizeWorld);
    void RenderAnimation(const AnimationComponent& animationInfo, glm::vec2 centerWorld, float angle);
    void RenderBackground(const BackgroundInfo& backgroundInfo);
};