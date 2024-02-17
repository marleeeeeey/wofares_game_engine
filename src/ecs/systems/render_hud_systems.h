#pragma once
#include <SDL2/SDL.h>
#include <ecs/components/game_components.h>
#include <ecs/components/game_state_component.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>

class HUDRenderSystem
{
    entt::registry& registry;
    SDL_Renderer* renderer;
    GameState& gameState;
public:
    HUDRenderSystem(entt::registry& registry, SDL_Renderer* renderer);
    void Render();
private:
    void RenderDebugMenu();
    void RenderGrid();
};