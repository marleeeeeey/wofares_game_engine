#pragma once
#include <SDL2/SDL.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <utils/game_options.h>

class RenderHUDSystem
{
    entt::registry& registry;
    SDL_Renderer* renderer;
    GameOptions& gameState;
    nlohmann::json assetsSettingsJson;
public:
    RenderHUDSystem(entt::registry& registry, SDL_Renderer* renderer, nlohmann::json assetsSettingsJson);
    void Render();
private:
    void RenderDebugMenu();
    void RenderGrid();
    void DrawPlayersWindowInfo();
    void ShowGameInstructions();
    void ShowLevelCompleteScreen(bool isWin);
};