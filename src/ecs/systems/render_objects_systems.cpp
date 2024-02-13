#include "render_objects_systems.h"
#include <ecs/components/all_components.h>
#include <numbers>
#include <utils/colors.h>
#include <utils/glm_box2d_conversions.h>
#include <utils/globals.h>

void RenderSystem(entt::registry& registry, SDL_Renderer* renderer)
{
    // Clear the screen with white color.
    SetRenderDrawColor(renderer, ColorName::White);
    SDL_RenderClear(renderer);

    auto& gameState = registry.get<GameState>(registry.view<GameState>().front());

    { // Render tiles.
        auto tilesView = registry.view<SizeComponent, TileInfo, PhysicalBody>();
        for (auto entity : tilesView)
        {
            const auto& [size, tileInfo, physicalBody] = tilesView.get<SizeComponent, TileInfo, PhysicalBody>(entity);
            b2Vec2 pos = physicalBody.value->GetBody()->GetPosition() * box2DtoSDL;
            float angle = physicalBody.value->GetBody()->GetAngle();

            // Compute the destination rectangle on the screen.
            glm::vec2 transformedPosition =
                (pos - gameState.renderingOptions.cameraCenter) * gameState.renderingOptions.cameraScale +
                gameState.renderingOptions.windowSize / 2.0f;

            // Have to render from the center of the object. Because the Box2D body is in the center of the object.
            SDL_Rect destRect = {
                static_cast<int>(transformedPosition.x - size.value.x * gameState.renderingOptions.cameraScale / 2),
                static_cast<int>(transformedPosition.y - size.value.y * gameState.renderingOptions.cameraScale / 2),
                static_cast<int>(size.value.x * gameState.renderingOptions.cameraScale),
                static_cast<int>(size.value.y * gameState.renderingOptions.cameraScale)};

            // Calculate the angle in degrees.
            SDL_Point center = {destRect.w / 2, destRect.h / 2};
            double angleDegrees = angle * 180.0 / std::numbers::pi;

            // Render the tile with the calculated angle.
            SDL_RenderCopyEx(
                renderer, tileInfo.texture->get(), &tileInfo.srcRect, &destRect, angleDegrees, &center, SDL_FLIP_NONE);
        }
    }

    { // Render players.
        SetRenderDrawColor(renderer, ColorName::Blue);
        auto players = registry.view<PhysicalBody, SizeComponent, PlayerNumber>();
        for (auto entity : players)
        {
            const auto& [physicalBody, size] = players.get<PhysicalBody, SizeComponent>(entity);
            const b2Vec2 pos = physicalBody.value->GetBody()->GetPosition() * box2DtoSDL;

            // TODO implement texture and angle for players.
            // float angle = physicalBody.value->GetBody()->GetAngle();

            glm::vec2 transformedPosition =
                (pos - gameState.renderingOptions.cameraCenter) * gameState.renderingOptions.cameraScale +
                gameState.renderingOptions.windowSize / 2.0f;

            // Have to render from the center of the object. Because the Box2D body is in the center of the object.
            SDL_Rect rect = {
                static_cast<int>(transformedPosition.x - size.value.x * gameState.renderingOptions.cameraScale / 2),
                static_cast<int>(transformedPosition.y - size.value.y * gameState.renderingOptions.cameraScale / 2),
                static_cast<int>(size.value.x * gameState.renderingOptions.cameraScale),
                static_cast<int>(size.value.y * gameState.renderingOptions.cameraScale)};

            SDL_RenderFillRect(renderer, &rect);
        }
    }
}
