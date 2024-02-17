#include "game_objects_renderer.h"
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
    RenderPlayerWeaponDirection();
}

void GameObjectsRenderer::RenderTiles()
{
    auto tilesView = registry.view<RenderingInfo, PhysicsInfo>();
    for (auto entity : tilesView)
    {
        const auto& [tileInfo, physicalBody] = tilesView.get<RenderingInfo, PhysicsInfo>(entity);
        RenderTiledSquare(physicalBody.bodyRAII, tileInfo);
    }
}

void GameObjectsRenderer::RenderPlayerWeaponDirection()
{
    auto players = registry.view<PhysicsInfo, RenderingInfo, PlayerInfo>();
    for (auto entity : players)
    {
        auto [physicalBody, renderingInfo, playerInfo] = players.get<PhysicsInfo, RenderingInfo, PlayerInfo>(entity);

        // Draw the weapon.
        const glm::vec2 playerSdlPos =
            coordinatesTransformer.PhysicsToWorld(physicalBody.bodyRAII->GetBody()->GetPosition());
        glm::vec2 weaponWorldSize = renderingInfo.sdlSize / 2.0f;
        glm::vec2 weaponWorldPos = playerSdlPos + playerInfo.weaponDirection * renderingInfo.sdlSize / 2;
        RenderSquare(weaponWorldPos, weaponWorldSize, ColorName::Red, 0);
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

void GameObjectsRenderer::RenderTiledSquare(std::shared_ptr<Box2dObjectRAII> body, const RenderingInfo& tileInfo)
{
    const glm::vec2 sdlPos = coordinatesTransformer.PhysicsToWorld(body->GetBody()->GetPosition());
    const float angle = body->GetBody()->GetAngle();

    auto sdlSize = tileInfo.sdlSize;
    SDL_Rect destRect = GetRectWithCameraTransform(sdlPos, sdlSize);

    if (!tileInfo.texturePtr)
    {
        SetRenderDrawColor(renderer, tileInfo.colorName);
        // TODO implement texture and angle support for non tiled objects.
        SDL_RenderFillRect(renderer, &destRect);
        return;
    }

    // Calculate the angle in degrees.
    SDL_Point center = {destRect.w / 2, destRect.h / 2};
    double angleDegrees = angle * 180.0 / std::numbers::pi;

    // Render the tile with the calculated angle.
    SDL_RenderCopyEx(
        renderer, tileInfo.texturePtr->get(), &tileInfo.textureRect, &destRect, angleDegrees, &center, SDL_FLIP_NONE);
}
