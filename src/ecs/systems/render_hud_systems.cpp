#include "render_hud_systems.h"
#include "ecs/components/all_components.h"
#include <imgui.h>
#include <my_common_cpp_utils/Logger.h>
#include <utils/colors.h>
#include <utils/draw.h>

void RenderHUDSystem(entt::registry& registry, SDL_Renderer* renderer)
{
    auto& gameState = registry.get<GameState>(registry.view<GameState>().front());

    ImGui::Begin("HUD");

    ImGui::Text(MY_FMT("Quit: {}", gameState.quit).c_str());
    ImGui::Text(MY_FMT("Window Size: {}", gameState.windowSize).c_str());
    ImGui::Text(MY_FMT("FPS: {}", gameState.fps).c_str());
    ImGui::Text(MY_FMT("Gravity: {:.2f}", gameState.physicsWorld->GetGravity().Length()).c_str());
    ImGui::Text(MY_FMT("World Scale: {:.2f}", gameState.cameraScale).c_str());
    ImGui::Text(MY_FMT("Camera Center: {}", gameState.cameraCenter).c_str());
    ImGui::Text(MY_FMT("Scene Captured: {}", gameState.isSceneCaptured).c_str());

    // Caclulare count of tiles:
    auto tiles = registry.view<TileInfo>();
    ImGui::Text(MY_FMT("Tiles number: {}", tiles.size()).c_str());

    // Calculate count of players:
    auto players = registry.view<PlayerNumber>();
    ImGui::Text(MY_FMT("Players number: {}", players.size()).c_str());

    ImGui::End();
}

void DrawGridSystem(SDL_Renderer* renderer, const GameState& gameState)
{
    const int gridSize = 32;
    const SDL_Color gridColor = GetSDLColor(ColorName::Grey);
    const SDL_Color screenCenterColor = GetSDLColor(ColorName::Red);
    const SDL_Color originColor = GetSDLColor(ColorName::Green);

    // Get the window size to determine the drawing area
    int windowWidth = static_cast<int>(gameState.windowSize.x);
    int windowHeight = static_cast<int>(gameState.windowSize.y);

    auto& cameraCenter = gameState.cameraCenter;

    // Calculate the start and end points for drawing the grid
    int startX = static_cast<int>(cameraCenter.x - windowWidth / 2 / gameState.cameraScale);
    int startY = static_cast<int>(cameraCenter.y - windowHeight / 2 / gameState.cameraScale);
    int endX = startX + windowWidth / gameState.cameraScale;
    int endY = startY + windowHeight / gameState.cameraScale;

    // Align the beginning of the grid with the cell boundaries and add 1.4 of the grid size to center the grid.
    // This shift is necessary to align Box2D bodies with the grid.
    startX -= startX % gridSize + gridSize / 4;
    startY -= startY % gridSize + gridSize / 4;

    // Draw vertical grid lines
    SetRenderDrawColor(renderer, gridColor);
    for (int x = startX; x <= endX; x += gridSize)
    {
        int screenX = static_cast<int>((x - cameraCenter.x) * gameState.cameraScale + windowWidth / 2);
        SDL_RenderDrawLine(renderer, screenX, 0, screenX, windowHeight);
    }

    // Draw horizontal grid lines
    for (int y = startY; y <= endY; y += gridSize)
    {
        int screenY = static_cast<int>((y - cameraCenter.y) * gameState.cameraScale + windowHeight / 2);
        SDL_RenderDrawLine(renderer, 0, screenY, windowWidth, screenY);
    }

    // Draw the center of screen point
    DrawCross(renderer, gameState.windowSize / 2.0f, 20, screenCenterColor);
}
