#include "game_objects_renderer.h"
#include <numbers>
#include <utils/glm_box2d_conversions.h>
#include <utils/globals.h>

GameObjectsRenderer::GameObjectsRenderer(entt::registry& registry, SDL_Renderer* renderer)
  : registry(registry), renderer(renderer), gameState(registry.get<GameState>(registry.view<GameState>().front()))
{
    // Clear the screen with white color.
    SetRenderDrawColor(renderer, ColorName::White);
    SDL_RenderClear(renderer);

    RenderTiles();
    RenderPlayers();
    RenderGranades();
}

void GameObjectsRenderer::RenderTiles()
{
    auto tilesView = registry.view<SizeComponent, TileInfo, PhysicalBody>();
    for (auto entity : tilesView)
    {
        const auto& [size, tileInfo, physicalBody] = tilesView.get<SizeComponent, TileInfo, PhysicalBody>(entity);
        RenderTiledSquare(physicalBody.value, size.value, tileInfo);
    }
}

void GameObjectsRenderer::RenderPlayers()
{
    auto players = registry.view<PhysicalBody, SizeComponent, PlayerNumber>();
    for (auto entity : players)
    {
        const auto& [physicalBody, size] = players.get<PhysicalBody, SizeComponent>(entity);
        RenderSquare(physicalBody.value, size.value, ColorName::Blue);
    }
}

void GameObjectsRenderer::RenderGranades()
{
    auto granades = registry.view<PhysicalBody, SizeComponent, Granade>();
    for (auto entity : granades)
    {
        const auto& [physicalBody, size] = granades.get<PhysicalBody, SizeComponent>(entity);
        RenderSquare(physicalBody.value, size.value, ColorName::Red);
    }
}

void GameObjectsRenderer::RenderSquare(std::shared_ptr<Box2dObjectRAII> body, const glm::vec2& sdlSize, ColorName color)
{
    const b2Vec2 pos = body->GetBody()->GetPosition() * box2DtoSDL;
    // TODO implement texture and angle support for non tiled objects.
    const float angle = body->GetBody()->GetAngle();

    SetRenderDrawColor(renderer, color);

    glm::vec2 transformedPosition =
        (pos - gameState.renderingOptions.cameraCenter) * gameState.renderingOptions.cameraScale +
        gameState.renderingOptions.windowSize / 2.0f;

    // Have to render from the center of the object. Because the Box2D body is in the center of the object.
    SDL_Rect rect = {
        static_cast<int>(transformedPosition.x - sdlSize.x * gameState.renderingOptions.cameraScale / 2),
        static_cast<int>(transformedPosition.y - sdlSize.y * gameState.renderingOptions.cameraScale / 2),
        static_cast<int>(sdlSize.x * gameState.renderingOptions.cameraScale),
        static_cast<int>(sdlSize.y * gameState.renderingOptions.cameraScale)};

    SDL_RenderFillRect(renderer, &rect);
}

void GameObjectsRenderer::RenderTiledSquare(
    std::shared_ptr<Box2dObjectRAII> body, const glm::vec2& sdlSize, const TileInfo& tileInfo)
{
    const b2Vec2 sdlPos = body->GetBody()->GetPosition() * box2DtoSDL;
    const float angle = body->GetBody()->GetAngle();

    // Compute the destination rectangle on the screen.
    glm::vec2 transformedPosition =
        (sdlPos - gameState.renderingOptions.cameraCenter) * gameState.renderingOptions.cameraScale +
        gameState.renderingOptions.windowSize / 2.0f;

    // Have to render from the center of the object. Because the Box2D body is in the center of the object.
    SDL_Rect destRect = {
        static_cast<int>(transformedPosition.x - sdlSize.x * gameState.renderingOptions.cameraScale / 2),
        static_cast<int>(transformedPosition.y - sdlSize.y * gameState.renderingOptions.cameraScale / 2),
        static_cast<int>(sdlSize.x * gameState.renderingOptions.cameraScale),
        static_cast<int>(sdlSize.y * gameState.renderingOptions.cameraScale)};

    // Calculate the angle in degrees.
    SDL_Point center = {destRect.w / 2, destRect.h / 2};
    double angleDegrees = angle * 180.0 / std::numbers::pi;

    // Render the tile with the calculated angle.
    SDL_RenderCopyEx(
        renderer, tileInfo.texture->get(), &tileInfo.srcRect, &destRect, angleDegrees, &center, SDL_FLIP_NONE);
}
