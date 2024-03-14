#include "game_objects_render_system.h"
#include <my_common_cpp_utils/logger.h>
#include <utils/glm_box2d_conversions.h>

GameObjectsRenderSystem::GameObjectsRenderSystem(
    entt::registry& registry, SDL_Renderer* renderer, ResourceManager& resourceManager,
    PrimitivesRenderer& primitivesRenderer)
  : registry(registry), renderer(renderer), resourceManager(resourceManager),
    gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())), coordinatesTransformer(registry),
    primitivesRenderer(primitivesRenderer)
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
    primitivesRenderer.RenderBackground(backgroundInfo);
};

void GameObjectsRenderSystem::RenderTiles()
{
    auto tilesView = registry.view<RenderingInfo, PhysicsInfo>();
    for (auto entity : tilesView)
    {
        const auto& [tileInfo, physicalBody] = tilesView.get<RenderingInfo, PhysicsInfo>(entity);
        const glm::vec2 sdlPos = coordinatesTransformer.PhysicsToWorld(physicalBody.bodyRAII->GetBody()->GetPosition());
        const float angle = physicalBody.bodyRAII->GetBody()->GetAngle();
        primitivesRenderer.RenderTiledSquare(sdlPos, angle, tileInfo);
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
        primitivesRenderer.RenderSquare(weaponWorldPos, weaponWorldSize, ColorName::Red, 0);
    }
}

void GameObjectsRenderSystem::RenderAnimations()
{
    auto view = registry.view<AnimationInfo, PhysicsInfo>();

    for (auto entity : view)
    {
        const auto& [animationInfo, physicsInfo] = view.get<AnimationInfo, PhysicsInfo>(entity);

        // Caclulate the position and angle of the animation.
        auto body = physicsInfo.bodyRAII->GetBody();
        glm::vec2 physicsBodyCenterWorld = coordinatesTransformer.PhysicsToWorld(body->GetPosition());
        const float angle = body->GetAngle();

        primitivesRenderer.RenderAnimation(animationInfo, physicsBodyCenterWorld, angle);
    }
}
