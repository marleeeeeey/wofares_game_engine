#include "game_objects_renderer.h"
#include "SDL_rect.h"
#include <numbers>
#include <utils/glm_box2d_conversions.h>

GameObjectsRenderer::GameObjectsRenderer(entt::registry& registry, SDL_Renderer* renderer)
  : registry(registry), renderer(renderer), gameState(registry.get<GameState>(registry.view<GameState>().front())),
    coordinatesTransformer(registry)
{
    // Clear the screen with white color.
    SetRenderDrawColor(renderer, ColorName::White);
    SDL_RenderClear(renderer);

    RenderTiles();
    RenderPlayers();
    RenderGranades();
    RenderBridges();
}

void GameObjectsRenderer::RenderTiles()
{
    auto tilesView = registry.view<SdlSizeComponent, TileInfo, PhysicalBody>();
    for (auto entity : tilesView)
    {
        const auto& [size, tileInfo, physicalBody] = tilesView.get<SdlSizeComponent, TileInfo, PhysicalBody>(entity);
        RenderTiledSquare(physicalBody.value, size.value, tileInfo);
    }
}
void GameObjectsRenderer::RenderPlayers()
{
    auto players = registry.view<PhysicalBody, SdlSizeComponent, PlayerNumber, PlayersWeaponDirection>();
    for (auto entity : players)
    {
        auto [physicalBody, playerSize, playerNumber, weaponDirection] =
            players.get<PhysicalBody, SdlSizeComponent, PlayerNumber, PlayersWeaponDirection>(entity);

        // Draw the player.
        glm::vec2 playerWorldPos = coordinatesTransformer.PhysicsToWorld(physicalBody.value->GetBody()->GetPosition());
        glm::vec2 playerWorldSize = playerSize.value;
        RenderSquare(playerWorldPos, playerWorldSize, ColorName::Blue, 0);

        // Draw the weapon.
        glm::vec2 weaponWorldSize = playerSize.value / 2.0f;
        glm::vec2 weaponWorldPos = playerWorldPos + weaponDirection.value * playerWorldSize / 2;
        RenderSquare(weaponWorldPos, weaponWorldSize, ColorName::Red, 0);
    }
}

void GameObjectsRenderer::RenderGranades()
{
    auto granades = registry.view<PhysicalBody, SdlSizeComponent, Granade>();
    for (auto entity : granades)
    {
        const auto& [physicalBody, size] = granades.get<PhysicalBody, SdlSizeComponent>(entity);
        RenderSquare(physicalBody.value, size.value, ColorName::Red);
    }
}

SDL_Rect GameObjectsRenderer::GetRectWithCameraTransform(const glm::vec2& sdlPos, const glm::vec2& sdlSize)
{
    auto& rOpt = gameState.windowOptions;

    glm::vec2 transformedPosition = (sdlPos - rOpt.cameraCenter) * rOpt.cameraScale + rOpt.windowSize / 2.0f;

    // Have to render from the center of the object. Because the Box2D body is in the center of the object.
    SDL_Rect rect = {
        static_cast<int>(transformedPosition.x - sdlSize.x * rOpt.cameraScale / 2),
        static_cast<int>(transformedPosition.y - sdlSize.y * rOpt.cameraScale / 2),
        static_cast<int>(sdlSize.x * rOpt.cameraScale), static_cast<int>(sdlSize.y * rOpt.cameraScale)};

    return rect;
}

void GameObjectsRenderer::RenderSquare(const glm::vec2& sdlPos, const glm::vec2& sdlSize, ColorName color, float angle)
{
    SetRenderDrawColor(renderer, color);
    SDL_Rect rect = GetRectWithCameraTransform(sdlPos, sdlSize);
    // TODO implement texture and angle support for non tiled objects.
    SDL_RenderFillRect(renderer, &rect);
}

void GameObjectsRenderer::RenderSquare(std::shared_ptr<Box2dObjectRAII> body, const glm::vec2& sdlSize, ColorName color)
{
    const glm::vec2 sdlPos = coordinatesTransformer.PhysicsToWorld(body->GetBody()->GetPosition());
    float angle = body->GetBody()->GetAngle();
    RenderSquare(sdlPos, sdlSize, color, angle);
}

void GameObjectsRenderer::RenderTiledSquare(
    std::shared_ptr<Box2dObjectRAII> body, const glm::vec2& sdlSize, const TileInfo& tileInfo)
{
    const glm::vec2 sdlPos = coordinatesTransformer.PhysicsToWorld(body->GetBody()->GetPosition());
    const float angle = body->GetBody()->GetAngle();

    SDL_Rect destRect = GetRectWithCameraTransform(sdlPos, sdlSize);

    // Calculate the angle in degrees.
    SDL_Point center = {destRect.w / 2, destRect.h / 2};
    double angleDegrees = angle * 180.0 / std::numbers::pi;

    // Render the tile with the calculated angle.
    SDL_RenderCopyEx(
        renderer, tileInfo.texture->get(), &tileInfo.srcRect, &destRect, angleDegrees, &center, SDL_FLIP_NONE);
}

void GameObjectsRenderer::RenderBridges()
{
    auto bridges = registry.view<PhysicalBody, SdlSizeComponent, Bridge>();
    for (auto entity : bridges)
    {
        const auto& [physicalBody, size] = bridges.get<PhysicalBody, SdlSizeComponent>(entity);
        RenderSquare(physicalBody.value, size.value, ColorName::Purple);
    }
}
