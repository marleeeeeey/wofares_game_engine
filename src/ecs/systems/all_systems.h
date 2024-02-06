#pragma once
#include "SDL_rect.h"
#include <SDL.h>
#include <cassert>
#include <ecs/components/all_components.h>
#include <entt/entt.hpp>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <imgui_impl_sdl2.h>
#include <map>
#include <my_common_cpp_utils/Logger.h>
#include <nlohmann/json.hpp>

enum class ColorName
{
    White,
    Red,
    Green,
    Blue,
    Black,
    Yellow,
    Cyan,
    Magenta,
    Orange,
    Purple,
    Grey,
    DarkGreen,
    Navy,
    Maroon,
    Olive,
    Teal,
    Silver,
    Lime,
    Aqua,
    Fuchsia,
};

SDL_Color GetSDLColor(ColorName colorName, Uint8 alpha = 255)
{
    static const std::map<ColorName, SDL_Color> colorMap = {
        {ColorName::White, {255, 255, 255, 255}},  {ColorName::Red, {255, 0, 0, 255}},
        {ColorName::Green, {0, 255, 0, 255}},      {ColorName::Blue, {0, 0, 255, 255}},
        {ColorName::Black, {0, 0, 0, 255}},        {ColorName::Yellow, {255, 255, 0, 255}},
        {ColorName::Cyan, {0, 255, 255, 255}},     {ColorName::Magenta, {255, 0, 255, 255}},
        {ColorName::Orange, {255, 165, 0, 255}},   {ColorName::Purple, {128, 0, 128, 255}},
        {ColorName::Grey, {128, 128, 128, 255}},   {ColorName::DarkGreen, {0, 100, 0, 255}},
        {ColorName::Navy, {0, 0, 128, 255}},       {ColorName::Maroon, {128, 0, 0, 255}},
        {ColorName::Olive, {128, 128, 0, 255}},    {ColorName::Teal, {0, 128, 128, 255}},
        {ColorName::Silver, {192, 192, 192, 255}}, {ColorName::Lime, {0, 255, 0, 255}},
        {ColorName::Aqua, {0, 255, 255, 255}},     {ColorName::Fuchsia, {255, 0, 255, 255}},
    };

    auto color = colorMap.at(colorName);
    color.a = alpha;

    return color;
}

void SetRenderDrawColor(SDL_Renderer* renderer, const SDL_Color& color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void SetRenderDrawColor(SDL_Renderer* renderer, ColorName colorName)
{
    SetRenderDrawColor(renderer, GetSDLColor(colorName));
}

void RenderSystem(entt::registry& registry, SDL_Renderer* renderer)
{
    SetRenderDrawColor(renderer, ColorName::White);
    SDL_RenderClear(renderer);

    auto& gameState = registry.get<GameState>(registry.view<GameState>().front());

    SetRenderDrawColor(renderer, ColorName::Red);
    auto objects = registry.view<Position, SizeComponent>();
    for (auto entity : objects)
    {
        const auto& [position, size] = objects.get<Position, SizeComponent>(entity);

        glm::vec2 transformedPosition =
            (position.value - gameState.cameraCenter) * gameState.cameraScale + gameState.windowSize / 2.0f;
        SDL_Rect rect = {
            static_cast<int>(transformedPosition.x), static_cast<int>(transformedPosition.y),
            static_cast<int>(size.value.x * gameState.cameraScale),
            static_cast<int>(size.value.y * gameState.cameraScale)};

        SDL_RenderFillRect(renderer, &rect);
    }

    // TODO: think how to make it more generic. Remove code duplication.
    SetRenderDrawColor(renderer, ColorName::Blue);
    auto players = registry.view<Position, SizeComponent, PlayerNumber>();
    for (auto entity : players)
    {
        const auto& [position, size] = players.get<Position, SizeComponent>(entity);

        glm::vec2 transformedPosition =
            (position.value - gameState.cameraCenter) * gameState.cameraScale + gameState.windowSize / 2.0f;
        SDL_Rect rect = {
            static_cast<int>(transformedPosition.x), static_cast<int>(transformedPosition.y),
            static_cast<int>(size.value.x * gameState.cameraScale),
            static_cast<int>(size.value.y * gameState.cameraScale)};

        SDL_RenderFillRect(renderer, &rect);
    }
}

void BoundarySystem(entt::registry& registry, const glm::vec2& windowSize)
{
    auto view = registry.view<Position, Velocity>();
    for (auto entity : view)
    {
        const auto& [position, velocity] = view.get<Position, Velocity>(entity);

        if (position.value.x < 0)
        {
            position.value.x = 0;
            velocity.value.x = 0;
        }
        else if (position.value.x > windowSize.x)
        {
            position.value.x = windowSize.x;
            velocity.value.x = 0;
        }

        if (position.value.y < 0)
        {
            position.value.y = 0;
            velocity.value.y = 0;
        }
        else if (position.value.y > windowSize.y)
        {
            position.value.y = windowSize.y;
            velocity.value.y = 0;
        }
    }
}

void InputSystem(entt::registry& registry)
{
    const float jumpVelocity = -600.0f;
    const float moveSpeed = 300.0f;

    const Uint8* currentKeyStates = SDL_GetKeyboardState(nullptr);

    auto playerWithVelocity = registry.view<PlayerNumber, Velocity>();
    for (auto entity : playerWithVelocity)
    {
        auto& vel = playerWithVelocity.get<Velocity>(entity);

        if (currentKeyStates[SDL_SCANCODE_UP])
        {
            vel.value.y = jumpVelocity;
        }
        if (currentKeyStates[SDL_SCANCODE_LEFT])
        {
            vel.value.x = -moveSpeed;
        }
        else if (currentKeyStates[SDL_SCANCODE_RIGHT])
        {
            vel.value.x = moveSpeed;
        }
        else
        {
            vel.value.x = 0;
        }
    }
}

void PhysicsSystem(entt::registry& registry, float deltaTime)
{
    auto gameStateEntity = registry.view<GameState>().front();
    const auto& gameState = registry.get<GameState>(gameStateEntity);

    auto view = registry.view<Position, Velocity>();
    for (auto entity : view)
    {
        auto& pos = view.get<Position>(entity).value;
        auto& vel = view.get<Velocity>(entity).value;

        vel.y += gameState.gravity * deltaTime;
        pos += vel * deltaTime;
    }
}

void EventSystem(entt::registry& registry, entt::dispatcher& dispatcher)
{
    auto& gameState = registry.get<GameState>(registry.view<GameState>().front());

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT)
        {
            gameState.quit = true;
        }

        else if (event.type == SDL_MOUSEWHEEL)
        {
            float prevScale = gameState.cameraScale;

            // Calculate the new scale of the camera
            const float scaleSpeed = 0.1f;
            if (event.wheel.y > 0)
                gameState.cameraScale += scaleSpeed;
            else if (event.wheel.y < 0)
                gameState.cameraScale -= scaleSpeed;
            gameState.cameraScale = glm::clamp(gameState.cameraScale, 0.2f, 3.0f);

            // Get the cursor coordinates in world coordinates
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            glm::vec2 mouseWorldBeforeZoom =
                (glm::vec2(mouseX, mouseY) - gameState.windowSize * 0.5f) / prevScale + gameState.cameraCenter;

            gameState.debugMsg2 = MY_FMT("mouseWorldBeforeZoom {}", mouseWorldBeforeZoom);

            // Calculate the new position of the camera so that the point under the cursor remains in the same place
            gameState.cameraCenter = mouseWorldBeforeZoom -
                (glm::vec2(mouseX, mouseY) - gameState.windowSize * 0.5f) / gameState.cameraScale;
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT)
        {
            gameState.isSceneCaptured = true;
        }
        else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_RIGHT)
        {
            gameState.isSceneCaptured = false;
        }
        else if (event.type == SDL_MOUSEMOTION && gameState.isSceneCaptured)
        {
            gameState.debugMsg = MY_FMT("Mouse moved to ({}, {})", event.motion.x, event.motion.y);

            float deltaX = event.motion.xrel / gameState.cameraScale;
            float deltaY = event.motion.yrel / gameState.cameraScale;
            gameState.cameraCenter.x -= deltaX;
            gameState.cameraCenter.y -= deltaY;
        }
    }
}

void RenderHUDSystem(entt::registry& registry, SDL_Renderer* renderer)
{
    auto& gameState = registry.get<GameState>(registry.view<GameState>().front());

    ImGui::Begin("HUD");

    ImGui::Text(MY_FMT("Quit: {}", gameState.quit).c_str());
    ImGui::Text(MY_FMT("Window Size: {}", gameState.windowSize).c_str());
    ImGui::Text(MY_FMT("FPS: {}", gameState.fps).c_str());
    ImGui::Text(MY_FMT("Gravity: {:.2f}", gameState.gravity).c_str());
    ImGui::Text(MY_FMT("World Scale: {:.2f}", gameState.cameraScale).c_str());
    ImGui::Text(MY_FMT("Camera Center: {}", gameState.cameraCenter).c_str());
    ImGui::Text(MY_FMT("Scene Captured: {}", gameState.isSceneCaptured).c_str());
    ImGui::Text(MY_FMT("Debug Message: {}", gameState.debugMsg).c_str());
    ImGui::Text(MY_FMT("Debug Message 2: {}", gameState.debugMsg2).c_str());

    // print player velocity
    auto playerWithVelocity = registry.view<PlayerNumber, Velocity>();
    for (auto entity : playerWithVelocity)
    {
        const auto& [vel, player] = playerWithVelocity.get<Velocity, PlayerNumber>(entity);
        ImGui::Text(MY_FMT("Player {} Velocity: {}", player.value, vel.value).c_str());
    }

    // caclulare count of entities with Position:
    auto positionEntities = registry.view<Position>();
    ImGui::Text(MY_FMT("Position Entities: {}", positionEntities.size()).c_str());

    if (ImGui::Button("Remove All Entities With Only Position"))
    {
        auto positionEntities = registry.view<Position>();

        for (auto entity : positionEntities)
        {
            if (!registry.any_of<Velocity>(entity))
            {
                registry.remove<Position>(entity);
            }
        }
    }

    ImGui::End();
}

void ScatterSystem(entt::registry& registry, const glm::vec2& windowSize)
{
    auto view = registry.view<Position>();
    for (auto entity : view)
    {
        auto& pos = view.get<Position>(entity);

        pos.value.x = glm::linearRand(0.0f, windowSize.x);
        pos.value.y = glm::linearRand(0.0f, windowSize.y);
    }
}

void DrawCross(SDL_Renderer* renderer, int centerX, int centerY, int size, const SDL_Color& color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLine(renderer, centerX - size / 2, centerY, centerX + size / 2, centerY);
    SDL_RenderDrawLine(renderer, centerX, centerY - size / 2, centerX, centerY + size / 2);
}

void DrawCross(SDL_Renderer* renderer, const glm::vec2& center, int size, const SDL_Color& color)
{
    DrawCross(renderer, static_cast<int>(center.x), static_cast<int>(center.y), size, color);
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

    // Align the beginning of the grid with the cell boundaries
    startX -= startX % gridSize;
    startY -= startY % gridSize;

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

struct AABB
{
    glm::vec2 min; // Left top corner
    glm::vec2 max; // Right upper corner
};

AABB СalculateAABB(const Position& position, const SizeComponent& size)
{
    AABB aabb;
    aabb.min = position.value;
    aabb.max = position.value + size.value;
    return aabb;
}

bool CheckAABBCollision(const AABB& a, const AABB& b)
{
    if (a.max.x < b.min.x || a.min.x > b.max.x)
        return false;
    if (a.max.y < b.min.y || a.min.y > b.max.y)
        return false;
    return true;
}

void CollisionSystem(entt::registry& registry)
{
    auto players = registry.view<Position, SizeComponent, Velocity, PlayerNumber>();
    auto objects = registry.view<Position, SizeComponent>();

    for (auto player : players)
    {
        const auto& [playerPos, playerSize, playerVel] = players.get<Position, SizeComponent, Velocity>(player);
        AABB playerAABB = СalculateAABB(playerPos, playerSize);

        for (auto object : objects)
        {
            if (player == object)
                continue;

            const auto& [objectPos, objectSize] = players.get<Position, SizeComponent>(object);
            AABB objectAABB = СalculateAABB(objectPos, objectSize);

            if (CheckAABBCollision(playerAABB, objectAABB))
            {
                glm::vec2 direction = objectPos.value - playerPos.value;
                float overlapX = 0.5f * (playerAABB.max.x - playerAABB.min.x + objectAABB.max.x - objectAABB.min.x) -
                    std::abs(direction.x);
                float overlapY = 0.5f * (playerAABB.max.y - playerAABB.min.y + objectAABB.max.y - objectAABB.min.y) -
                    std::abs(direction.y);

                if (overlapX < overlapY)
                {
                    playerPos.value.x -= overlapX * glm::sign(playerVel.value.x);
                    playerVel.value.x = 0;
                }
                else
                {
                    playerPos.value.y -= overlapY * glm::sign(playerVel.value.y);
                    playerVel.value.y = 0;
                }
            }
        }
    }
}

void LoadMap(entt::registry& registry, const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Failed to open map file");

    nlohmann::json json;
    file >> json;

    // Assume all tiles are of the same size and the map is not infinite
    int tileWidth = json["tilewidth"];
    int tileHeight = json["tileheight"];
    glm::vec2 tileSize{tileWidth, tileHeight};

    // Iterate over each tile layer
    size_t createdTiles = 0;
    for (const auto& layer : json["layers"])
    {
        if (layer["type"] == "tilelayer")
        {
            int width = layer["width"];
            int height = layer["height"];
            const auto& tiles = layer["data"];

            // Create entities for each tile
            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    int tileId = tiles[x + y * width];
                    if (tileId > 0)
                    { // Skip empty tiles
                        auto entity = registry.create();
                        registry.emplace<Position>(entity, glm::vec2(x * tileWidth, y * tileHeight));
                        registry.emplace<SizeComponent>(entity, tileSize);
                        createdTiles++;
                        // TODO Additional components can be added here, such as tile type if needed
                    }
                }
            }
        }
    }

    if (createdTiles == 0)
        throw std::runtime_error(MY_FMT("No tiles were created during map loading {}", filename));
}
