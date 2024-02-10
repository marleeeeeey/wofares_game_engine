#include "render_objects_systems.h"
#include <ecs/components/all_components.h>
#include <utils/colors.h>

void RenderSystem(entt::registry& registry, SDL_Renderer* renderer)
{
    // Clear the screen with white color.
    SetRenderDrawColor(renderer, ColorName::White);
    SDL_RenderClear(renderer);

    auto& gameState = registry.get<GameState>(registry.view<GameState>().front());

    { // Render tiles.
        auto tilesView = registry.view<Position, SizeComponent, TileInfo, Renderable>();
        for (auto entity : tilesView)
        {
            auto& position = tilesView.get<Position>(entity);
            auto& size = tilesView.get<SizeComponent>(entity);
            auto& renderable = tilesView.get<Renderable>(entity);

            // Compute the destination rectangle on the screen.
            glm::vec2 transformedPosition =
                (position.value - gameState.cameraCenter) * gameState.cameraScale + gameState.windowSize / 2.0f;
            SDL_Rect destRect = {
                static_cast<int>(transformedPosition.x), static_cast<int>(transformedPosition.y),
                static_cast<int>(size.value.x * gameState.cameraScale),
                static_cast<int>(size.value.y * gameState.cameraScale)};

            // Render the tile.
            SDL_RenderCopy(renderer, renderable.texture->get(), &renderable.srcRect, &destRect);
        }
    }

    { // Render players.
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
}
