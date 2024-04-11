#include "render_hud_systems.h"
#include <ecs/components/physics_components.h>
#include <ecs/components/player_components.h>
#include <ecs/components/rendering_components.h>
#include <imgui.h>
#include <my_cpp_utils/config.h>
#include <my_cpp_utils/logger.h>
#include <utils/RAII/imgui_RAII.h>
#include <utils/game_options.h>
#include <utils/sdl_colors.h>
#include <utils/sdl_draw.h>

RenderHUDSystem::RenderHUDSystem(entt::registry& registry, SDL_Renderer* renderer)
  : registry(registry), renderer(renderer), gameState(registry.get<GameOptions>(registry.view<GameOptions>().front()))
{}

void RenderHUDSystem::Render()
{
    if (utils::GetConfig<bool, "HUDRenderSystem.showGrid">())
        RenderGrid();

    RenderDebugMenu();
    DrawPlayersWindowInfo();
}

void RenderHUDSystem::RenderDebugMenu()
{
    ImGui::Begin("Debug Menu");

    if (ImGui::Button("Reload Map"))
        gameState.controlOptions.reloadMap = true;

    // Caclulare count of tiles, players and dynamic bodies:
    auto tiles = registry.view<RenderingComponent>();
    auto players = registry.view<PlayerComponent>();
    auto dynamicBodies = registry.view<PhysicsComponent>();
    size_t dynamicBodiesCount = 0;
    for (auto entity : dynamicBodies)
    {
        auto body = dynamicBodies.get<PhysicsComponent>(entity).bodyRAII->GetBody();
        if (body->GetType() == b2_dynamicBody)
            dynamicBodiesCount++;
    }

    // Print the gravity and camera scale, count of tiles, players and dynamic bodies in one line.
    auto gravity = gameState.physicsWorld->GetGravity().Length();
    auto cameraScale = gameState.windowOptions.cameraScale;
    ImGui::Text(MY_FMT("{:.2f}/{:.2f} (Gr/Sc)", gravity, cameraScale).c_str());
    ImGui::Text(MY_FMT("{}/{}/{} (Ts/Ps/DB)", tiles.size(), players.size(), dynamicBodiesCount).c_str());
    ImGui::Text(MY_FMT("Camera center: {}", gameState.windowOptions.cameraCenterSdl).c_str());

    // Print debug info.
    ImGui::Text(MY_FMT("Space pressed duration: {:.2f}", gameState.debugInfo.spacePressedDuration).c_str());
    ImGui::Text(MY_FMT("Space pressed duration on up event: {:.2f}", gameState.debugInfo.spacePressedDurationOnUpEvent)
                    .c_str());

    // Print last mouse position.
    const auto& lastMousePosition = gameState.windowOptions.lastMousePosInWindow;
    ImGui::Text(MY_FMT("Last mouse position: ({:.2f}, {:.2f})", lastMousePosition.x, lastMousePosition.y).c_str());

    ImGui::End();
}

void RenderHUDSystem::RenderGrid()
{
    auto& gameState = registry.get<GameOptions>(registry.view<GameOptions>().front());

    const int gridSize = 32;
    const SDL_Color gridColor = GetSDLColor(ColorName::Grey);
    const SDL_Color screenCenterColor = GetSDLColor(ColorName::Red);

    // Get the window size to determine the drawing area
    int windowWidth = static_cast<int>(gameState.windowOptions.windowSize.x);
    int windowHeight = static_cast<int>(gameState.windowOptions.windowSize.y);

    const auto& cameraCenterSdl = gameState.windowOptions.cameraCenterSdl;

    // Calculate the start and end points for drawing the grid
    int startX = static_cast<int>(cameraCenterSdl.x - windowWidth / 2 / gameState.windowOptions.cameraScale);
    int startY = static_cast<int>(cameraCenterSdl.y - windowHeight / 2 / gameState.windowOptions.cameraScale);
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
        int screenX = static_cast<int>((x - cameraCenterSdl.x) * gameState.windowOptions.cameraScale + windowWidth / 2);
        SDL_RenderDrawLine(renderer, screenX, 0, screenX, windowHeight);
    }

    // Draw horizontal grid lines
    for (int y = startY; y <= endY; y += gridSize)
    {
        int screenY =
            static_cast<int>((y - cameraCenterSdl.y) * gameState.windowOptions.cameraScale + windowHeight / 2);
        SDL_RenderDrawLine(renderer, 0, screenY, windowWidth, screenY);
    }

    // Draw the center of screen point
    DrawCross(renderer, gameState.windowOptions.windowSize / 2.0f, 20, screenCenterColor);
}

void RenderHUDSystem::DrawPlayersWindowInfo()
{
    for (auto entity : registry.view<PlayerComponent>())
    {
        auto& playerInfo = registry.get<PlayerComponent>(entity);
        ImGui::Begin(MY_FMT("Player {} HUD", playerInfo.number).c_str());

        // Encrease font size.
        ImGui::SetWindowFontScale(3);

        std::string reloadingStatus = "";
        if (playerInfo.weapons.at(playerInfo.currentWeapon).remainingReloadTime > 0)
            reloadingStatus = "Reloading...";
        else if (playerInfo.weapons.at(playerInfo.currentWeapon).remainingFireRate > 0)
            reloadingStatus = "Fire rate...";

        ImGui::Text(MY_FMT(
                        "{}, Ammo: {}/{} {}", playerInfo.currentWeapon,
                        playerInfo.weapons.at(playerInfo.currentWeapon).ammoInClip,
                        playerInfo.weapons.at(playerInfo.currentWeapon).ammoInStorage, reloadingStatus)
                        .c_str());

        ImGui::End();
    }
};