#pragma once
#include <SDL2/SDL.h>
#include <ecs/components/game_components.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <utils/game_options.h>


class HUDRenderSystem
{
    entt::registry& registry;
    SDL_Renderer* renderer;
    GameOptions& gameState;
public:
    HUDRenderSystem(entt::registry& registry, SDL_Renderer* renderer);
    void Render();
private:
    void RenderDebugMenu();
    void RenderGrid();
};