#include "sdl_primitives_renderer.h"
#include <numbers>

SdlPrimitivesRenderer::SdlPrimitivesRenderer(
    entt::registry& registry, SDL_Renderer* renderer, ResourceManager& resourceManager)
  : registry(registry), renderer(renderer), resourceManager(resourceManager),
    gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())), coordinatesTransformer(registry)
{}

SDL_Rect SdlPrimitivesRenderer::GetRectWithCameraTransform(const glm::vec2& posWorld, const glm::vec2& sizeWorld)
{
    auto& rOpt = gameState.windowOptions;

    glm::vec2 transformedPosition = (posWorld - rOpt.cameraCenterSdl) * rOpt.cameraScale + rOpt.windowSize / 2.0f;

    // Have to render from the center of the object. Because the Box2D body is in the center of the object.
    SDL_Rect rect = {
        static_cast<int>(transformedPosition.x - sizeWorld.x * rOpt.cameraScale / 2),
        static_cast<int>(transformedPosition.y - sizeWorld.y * rOpt.cameraScale / 2),
        static_cast<int>(sizeWorld.x * rOpt.cameraScale), static_cast<int>(sizeWorld.y * rOpt.cameraScale)};

    return rect;
}

void SdlPrimitivesRenderer::RenderSquare(
    const glm::vec2& posWorld, const glm::vec2& sizeWorld, ColorName color, float angle)
{
    std::shared_ptr<SDLTextureRAII> pixelTexture = resourceManager.GetColoredPixelTexture(color);
    double angleDegrees = angle * (180.0 / M_PI);
    SDL_Rect destRect = GetRectWithCameraTransform(posWorld, sizeWorld);
    SDL_Point center = {destRect.w / 2, destRect.h / 2};
    SDL_RenderCopyEx(renderer, pixelTexture->get(), nullptr, &destRect, angleDegrees, &center, SDL_FLIP_NONE);
}

void SdlPrimitivesRenderer::RenderSquare(
    std::shared_ptr<Box2dObjectRAII> body, const glm::vec2& sizeWorld, ColorName color)
{
    const glm::vec2 posWorld = coordinatesTransformer.PhysicsToWorld(body->GetBody()->GetPosition());
    float angle = body->GetBody()->GetAngle();
    RenderSquare(posWorld, sizeWorld, color, angle);
}

void SdlPrimitivesRenderer::RenderCircle(const glm::vec2& centerWorld, float radiusWorld, ColorName color)
{
    // TODO4: Now the circle is rendered as a square. Fix it.
    std::shared_ptr<SDLTextureRAII> pixelTexture = resourceManager.GetColoredPixelTexture(color);
    SDL_Rect destRect = GetRectWithCameraTransform(centerWorld, glm::vec2(radiusWorld * 2, radiusWorld * 2));
    SDL_RenderCopy(renderer, pixelTexture->get(), nullptr, &destRect);
}

void SdlPrimitivesRenderer::RenderTiledSquare(
    const glm::vec2& centerWorld, const float angle, const RenderingComponent& tileInfo, const SDL_RendererFlip& flip)
{
    auto sizeWorld = tileInfo.sizeWorld;
    SDL_Rect destRect = GetRectWithCameraTransform(centerWorld, sizeWorld);

    if (!tileInfo.texturePtr)
    {
        RenderSquare(centerWorld, sizeWorld, tileInfo.colorName, angle);
        return;
    }

    // Calculate the angle in degrees.
    SDL_Point center = {destRect.w / 2, destRect.h / 2};
    double angleDegrees = angle * 180.0 / std::numbers::pi;

    // Render the tile with the calculated angle.
    SDL_RenderCopyEx(
        renderer, tileInfo.texturePtr->get(), &tileInfo.textureRect, &destRect, angleDegrees, &center, flip);
}

void SdlPrimitivesRenderer::RenderAnimation(const AnimationComponent& animationInfo, glm::vec2 centerWorld, float angle)
{
    if (animationInfo.animation.frames.empty())
        return;

    // TODO1: Unify using the modulo operator in interface.
    auto safeIndex = animationInfo.currentFrameIndex % animationInfo.animation.frames.size();

    const auto& animation = animationInfo.animation;
    const auto& frame = animation.frames[safeIndex];

    MY_LOG(
        trace, "safeIndex: {}, textureRect: x: {}, y: {}, w: {}, h: {}", safeIndex, frame.renderingInfo.textureRect.x,
        frame.renderingInfo.textureRect.y, frame.renderingInfo.textureRect.w, frame.renderingInfo.textureRect.h);

    if (animation.hitboxRect)
    {
        const SDL_Rect& hitboxRect = *animation.hitboxRect;
        auto textureSize = frame.renderingInfo.sizeWorld;

        // Shift the center of the animation to the center of the hitbox.
        auto textureCenter = textureSize / 2.0f;
        auto hitboxCenter = glm::vec2(hitboxRect.x, hitboxRect.y) + glm::vec2(hitboxRect.w, hitboxRect.h) / 2.0f;
        auto shift = hitboxCenter - textureCenter;
        auto hitboxNewCenter = centerWorld - shift;
        centerWorld = hitboxNewCenter;
    }

    RenderTiledSquare(centerWorld, angle, frame.renderingInfo, animationInfo.flip);
}

void SdlPrimitivesRenderer::RenderAnimationFirstFrame(
    const Animation& animation, glm::vec2 centerWorld, float angle, const SDL_RendererFlip& flip)
{
    if (animation.frames.empty())
        return;

    const auto& frame = animation.frames[0];

    MY_LOG(
        trace, "textureRect: x: {}, y: {}, w: {}, h: {}", frame.renderingInfo.textureRect.x,
        frame.renderingInfo.textureRect.y, frame.renderingInfo.textureRect.w, frame.renderingInfo.textureRect.h);

    RenderTiledSquare(centerWorld, angle, frame.renderingInfo, flip);
}

void SdlPrimitivesRenderer::RenderBackground(const BackgroundInfo& backgroundInfo)
{
    auto textureRAII = backgroundInfo.texture;
    if (!textureRAII)
    {
        MY_LOG(warn, "No background texture to render");
        return;
    }
    auto backgroundTexture = textureRAII->get();

    auto textureScale = backgroundInfo.textureScale;

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
}
