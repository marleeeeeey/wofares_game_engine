#include "render_hud_systems.h"
#include <ecs/components/game_components.h>
#include <ecs/components/game_state_component.h>
#include <imgui.h>
#include <my_common_cpp_utils/Logger.h>
#include <utils/imgui_RAII.h>
#include <utils/sdl_colors.h>
#include <utils/sdl_draw.h>

HUDRenderSystem::HUDRenderSystem(entt::registry& registry, SDL_Renderer* renderer)
  : registry(registry), renderer(renderer), gameState(registry.get<GameState>(registry.view<GameState>().front()))
{}

void HUDRenderSystem::Render()
{
    RenderGrid();
    RenderDebugMenu();
}

void HUDRenderSystem::RenderDebugMenu()
{
    ImGui::Begin("HUD");

    if (ImGui::Button("Reload Map"))
        gameState.controlOptions.reloadMap = true;

    // Caclulare count of tiles, players and dynamic bodies:
    auto tiles = registry.view<RenderingInfo>();
    auto players = registry.view<PlayerInfo>();
    auto dynamicBodies = registry.view<PhysicsInfo>();
    size_t dynamicBodiesCount = 0;
    for (auto entity : dynamicBodies)
    {
        auto body = dynamicBodies.get<PhysicsInfo>(entity).bodyRAII->GetBody();
        if (body->GetType() == b2_dynamicBody)
            dynamicBodiesCount++;
    }

    // Print the gravity and camera scale, count of tiles, players and dynamic bodies in one line.
    auto gravity = gameState.physicsWorld->GetGravity().Length();
    auto cameraScale = gameState.windowOptions.cameraScale;
    ImGui::Text(MY_FMT("{:.2f}/{:.2f} (Gr/Sc)", gravity, cameraScale).c_str());
    ImGui::Text(MY_FMT("{}/{}/{} (Ts/Ps/DB)", tiles.size(), players.size(), dynamicBodiesCount).c_str());

    // Print debug info.
    ImGui::Text(MY_FMT("Space pressed duration: {:.2f}", gameState.debugInfo.spacePressedDuration).c_str());
    ImGui::Text(MY_FMT("Space pressed duration on up event: {:.2f}", gameState.debugInfo.spacePressedDurationOnUpEvent)
                    .c_str());

    // Print last mouse position.
    const auto& lastMousePosition = gameState.windowOptions.lastMousePosInWindow;
    ImGui::Text(MY_FMT("Last mouse position: ({:.2f}, {:.2f})", lastMousePosition.x, lastMousePosition.y).c_str());

    if (ImGui::CollapsingHeader("Advanced Options"))
    {
        // Draw controls the physics world.
        ImGui::SliderInt("Velocity Iterations", (int*)&gameState.physicsOptions.velocityIterations, 1, 10);
        ImGui::SliderInt("Position Iterations", (int*)&gameState.physicsOptions.positionIterations, 1, 10);
        ImGui::SliderInt("Mini Tile Resolution", (int*)&gameState.levelOptions.miniTileResolution, 1, 8);
        ImGui::SliderFloat("Dynamic Body Probability", &gameState.levelOptions.dynamicBodyProbability, 0.0f, 1.0f);
        ImGui::SliderFloat(
            "Gap Between Physical And Visual", &gameState.physicsOptions.gapBetweenPhysicalAndVisual, 0.0f, 1.0f);
        ImGui::Checkbox("Prevent Creation Invisible Tiles", &gameState.levelOptions.preventCreationInvisibleTiles);

        // box2DtoSDL setting.
        float box2DtoSdlStep = 4.0f;
        float& box2DtoSDL = gameState.windowOptions.box2DtoSDL;
        int intValue = static_cast<int>(box2DtoSDL / box2DtoSdlStep);
        if (ImGui::SliderInt(MY_FMT("Box2D to SDL coef, x{:.2f}", box2DtoSdlStep).c_str(), &intValue, 8, 14))
            box2DtoSDL = intValue * box2DtoSdlStep;
        ImGui::Text(MY_FMT("Box2D to SDL: {:.2f}", box2DtoSDL).c_str());
    }

    ImGui::End();
}

void HUDRenderSystem::RenderGrid()
{
    auto& gameState = registry.get<GameState>(registry.view<GameState>().front());

    const int gridSize = 32;
    const SDL_Color gridColor = GetSDLColor(ColorName::Grey);
    const SDL_Color screenCenterColor = GetSDLColor(ColorName::Red);
    const SDL_Color originColor = GetSDLColor(ColorName::Green);

    // Get the window size to determine the drawing area
    int windowWidth = static_cast<int>(gameState.windowOptions.windowSize.x);
    int windowHeight = static_cast<int>(gameState.windowOptions.windowSize.y);

    auto& cameraCenter = gameState.windowOptions.cameraCenter;

    // Calculate the start and end points for drawing the grid
    int startX = static_cast<int>(cameraCenter.x - windowWidth / 2 / gameState.windowOptions.cameraScale);
    int startY = static_cast<int>(cameraCenter.y - windowHeight / 2 / gameState.windowOptions.cameraScale);
    int endX = startX + windowWidth / gameState.windowOptions.cameraScale;
    int endY = startY + windowHeight / gameState.windowOptions.cameraScale;

    // Align the beginning of the grid with the cell boundaries and add 1.4 of the grid size to center the grid.
    // This shift is necessary to align Box2D bodies with the grid.
    startX -= startX % gridSize + gridSize / 4;
    startY -= startY % gridSize + gridSize / 4;

    // Draw vertical grid lines
    SetRenderDrawColor(renderer, gridColor);
    for (int x = startX; x <= endX; x += gridSize)
    {
        int screenX = static_cast<int>((x - cameraCenter.x) * gameState.windowOptions.cameraScale + windowWidth / 2);
        SDL_RenderDrawLine(renderer, screenX, 0, screenX, windowHeight);
    }

    // Draw horizontal grid lines
    for (int y = startY; y <= endY; y += gridSize)
    {
        int screenY = static_cast<int>((y - cameraCenter.y) * gameState.windowOptions.cameraScale + windowHeight / 2);
        SDL_RenderDrawLine(renderer, 0, screenY, windowWidth, screenY);
    }

    // Draw the center of screen point
    DrawCross(renderer, gameState.windowOptions.windowSize / 2.0f, 20, screenCenterColor);
}
