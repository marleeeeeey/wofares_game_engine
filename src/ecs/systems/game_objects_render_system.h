#pragma once
#include <SDL.h>
#include <entt/entt.hpp>
#include <utils/coordinates_transformer.h>
#include <utils/resources/resource_manager.h>
#include <utils/sdl_colors.h>
#include <utils/sdl_primitives_renderer.h>

class GameObjectsRenderSystem
{
    entt::registry& registry;
    SDL_Renderer* renderer;
    ResourceManager& resourceManager;
    GameOptions& gameState;
    CoordinatesTransformer coordinatesTransformer;
    SdlPrimitivesRenderer& primitivesRenderer;
public:
    GameObjectsRenderSystem(
        entt::registry& registry, SDL_Renderer* renderer, ResourceManager& resourceManager,
        SdlPrimitivesRenderer& primitivesRenderer);
    void Render();
private: // Render game objects methods.
    void RenderBackground();
    void RenderTiles();
    void RenderAnimations();
    void RenderPlayerWeaponDirection();
};
