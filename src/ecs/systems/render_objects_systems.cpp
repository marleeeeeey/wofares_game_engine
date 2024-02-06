#include "render_objects_systems.h"
#include <ecs/components/all_components.h>
#include <utils/colors.h>

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
