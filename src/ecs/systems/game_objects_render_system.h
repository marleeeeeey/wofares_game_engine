#pragma once
#include <SDL.h>
#include <entt/entt.hpp>
#include <utils/coordinates_transformer.h>
#include <utils/primitives_renderer.h>
#include <utils/resources/resource_manager.h>
#include <utils/sdl_colors.h>

class GameObjectsRenderSystem
{
    entt::registry& registry;
    SDL_Renderer* renderer;
    ResourceManager& resourceManager;
    GameOptions& gameState;
    CoordinatesTransformer coordinatesTransformer;
    PrimitivesRenderer& primitivesRenderer;
public:
    GameObjectsRenderSystem(
        entt::registry& registry, SDL_Renderer* renderer, ResourceManager& resourceManager,
        PrimitivesRenderer& primitivesRenderer);
    void Render();
private: // Render game objects methods.
    void RenderBackground();
    void RenderTiles();
    void RenderAnimations();
    void RenderPlayerWeaponDirection();
};
