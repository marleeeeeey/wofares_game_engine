#include "game_objects_render_system.h"
#include <my_common_cpp_utils/logger.h>
#include <numbers>
#include <utils/glm_box2d_conversions.h>

GameObjectsRenderSystem::GameObjectsRenderSystem(
    entt::registry& registry, SDL_Renderer* renderer, ResourceManager& resourceManager)
  : registry(registry), renderer(renderer), resourceManager(resourceManager),
    gameState(registry.get<GameState>(registry.view<GameState>().front())), coordinatesTransformer(registry)
{}

void GameObjectsRenderSystem::Render()
{
    // Clear the screen with white color.
    SetRenderDrawColor(renderer, ColorName::Black);
    SDL_RenderClear(renderer);

    RenderBackground();

    RenderTiles();
    RenderAnimations();
    RenderPlayerWeaponDirection();
};

void GameObjectsRenderSystem::RenderBackground()
{
    auto backgroundInfo = gameState.levelOptions.backgroundInfo;
    auto textureRAII = backgroundInfo.texture;
    if (!textureRAII)
    {
        MY_LOG_FMT(warn, "No background texture to render");
        return;
    }
    auto backgroundTexture = textureRAII->get();

    auto textureScale = backgroundInfo.textureScale;
    auto backgroundSpeed = backgroundInfo.speedFactor;

    int textureWidth, textureHeight;
    SDL_QueryTexture(backgroundTexture, nullptr, nullptr, &textureWidth, &textureHeight);

    // Calculate the size of the texture after scaling.
    textureWidth *= textureScale;
    textureHeight *= textureScale;

    // Calculate the background offset depending on the camera position and the "depth" of the background.
    auto backgroundCenter = gameState.windowOptions.cameraCenterSdl * backgroundInfo.speedFactor;
    auto backgroundTopLeft = backgroundCenter - glm::vec2(textureWidth, textureHeight) / 2.0f;

    // Set the destination rectangle for the texture.
    SDL_Rect dstRect;
    dstRect.x = backgroundTopLeft.x;
    dstRect.y = backgroundTopLeft.y;
    dstRect.w = textureWidth;
    dstRect.h = textureHeight;

    // Render the background texture.
    SDL_RenderCopy(renderer, backgroundTexture, nullptr, &dstRect);
};

void GameObjectsRenderSystem::RenderTiles()
{
    auto tilesView = registry.view<RenderingInfo, PhysicsInfo>();
    for (auto entity : tilesView)
    {
        const auto& [tileInfo, physicalBody] = tilesView.get<RenderingInfo, PhysicsInfo>(entity);
        RenderTiledSquare(physicalBody.bodyRAII, tileInfo);
    }
}

void GameObjectsRenderSystem::RenderPlayerWeaponDirection()
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

SDL_Rect GameObjectsRenderSystem::GetRectWithCameraTransform(const glm::vec2& sdlPos, const glm::vec2& sdlSize)
{
    auto& rOpt = gameState.windowOptions;

    glm::vec2 transformedPosition = (sdlPos - rOpt.cameraCenterSdl) * rOpt.cameraScale + rOpt.windowSize / 2.0f;

    // Have to render from the center of the object. Because the Box2D body is in the center of the object.
    SDL_Rect rect = {
        static_cast<int>(transformedPosition.x - sdlSize.x * rOpt.cameraScale / 2),
        static_cast<int>(transformedPosition.y - sdlSize.y * rOpt.cameraScale / 2),
        static_cast<int>(sdlSize.x * rOpt.cameraScale), static_cast<int>(sdlSize.y * rOpt.cameraScale)};

    return rect;
}

void GameObjectsRenderSystem::RenderSquare(
    const glm::vec2& sdlPos, const glm::vec2& sdlSize, ColorName color, float angle)
{
    std::shared_ptr<SDLTextureRAII> pixelTexture = resourceManager.GetColoredPixelTexture(color);
    double angleDegrees = angle * (180.0 / M_PI);
    SDL_Rect destRect = GetRectWithCameraTransform(sdlPos, sdlSize);
    SDL_Point center = {destRect.w / 2, destRect.h / 2};
    SDL_RenderCopyEx(renderer, pixelTexture->get(), nullptr, &destRect, angleDegrees, &center, SDL_FLIP_NONE);
}

void GameObjectsRenderSystem::RenderSquare(
    std::shared_ptr<Box2dObjectRAII> body, const glm::vec2& sdlSize, ColorName color)
{
    const glm::vec2 sdlPos = coordinatesTransformer.PhysicsToWorld(body->GetBody()->GetPosition());
    float angle = body->GetBody()->GetAngle();
    RenderSquare(sdlPos, sdlSize, color, angle);
}

void GameObjectsRenderSystem::RenderTiledSquare(
    std::shared_ptr<Box2dObjectRAII> body, const RenderingInfo& tileInfo, const SDL_RendererFlip& flip)
{
    const glm::vec2 sdlPos = coordinatesTransformer.PhysicsToWorld(body->GetBody()->GetPosition());
    const float angle = body->GetBody()->GetAngle();

    auto sdlSize = tileInfo.sdlSize;
    SDL_Rect destRect = GetRectWithCameraTransform(sdlPos, sdlSize);

    if (!tileInfo.texturePtr)
    {
        RenderSquare(sdlPos, sdlSize, tileInfo.colorName, angle);
        return;
    }

    // Calculate the angle in degrees.
    SDL_Point center = {destRect.w / 2, destRect.h / 2};
    double angleDegrees = angle * 180.0 / std::numbers::pi;

    // Render the tile with the calculated angle.
    SDL_RenderCopyEx(
        renderer, tileInfo.texturePtr->get(), &tileInfo.textureRect, &destRect, angleDegrees, &center, flip);
}

void GameObjectsRenderSystem::RenderAnimations()
{
    auto view = registry.view<AnimationInfo, PhysicsInfo>();

    for (auto entity : view)
    {
        const auto& [animation, body] = view.get<AnimationInfo, PhysicsInfo>(entity);

        if (animation.isPlaying && !animation.frames.empty())
        {
            const auto& frame = animation.frames[animation.currentFrameIndex];
            const auto& renderingInfo = frame.renderingInfo;
            RenderTiledSquare(body.bodyRAII, renderingInfo, animation.flip);
        }
    }
}
