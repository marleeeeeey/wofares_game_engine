#include "game_objects_render_system.h"
#include <SDL_render.h>
#include <ecs/components/animation_components.h>
#include <ecs/components/physics_components.h>
#include <ecs/components/player_components.h>
#include <my_cpp_utils/config.h>
#include <my_cpp_utils/logger.h>
#include <utils/glm_box2d_conversions.h>

GameObjectsRenderSystem::GameObjectsRenderSystem(
    entt::registry& registry, SDL_Renderer* renderer, ResourceManager& resourceManager,
    SdlPrimitivesRenderer& primitivesRenderer)
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
    auto tilesView = registry.view<RenderingComponent, PhysicsComponent>();
    for (auto entity : tilesView)
    {
        const auto& [tileInfo, physicalBody] = tilesView.get<RenderingComponent, PhysicsComponent>(entity);
        const glm::vec2 posWorld =
            coordinatesTransformer.PhysicsToWorld(physicalBody.bodyRAII->GetBody()->GetPosition());
        const float angle = physicalBody.bodyRAII->GetBody()->GetAngle();
        primitivesRenderer.RenderTiledSquare(posWorld, angle, tileInfo);
    }
}

void GameObjectsRenderSystem::RenderPlayerWeaponDirection()
{
    auto players = registry.view<PhysicsComponent, PlayerComponent, AnimationComponent>();
    for (auto entity : players)
    {
        auto [physicalBody, playerInfo, animationComponent] =
            players.get<PhysicsComponent, PlayerComponent, AnimationComponent>(entity);

        // Draw the weapon.
        // TODO1: Currently we are always get the animation in initial state. So it always draws the first frame.
        // We should use AnimationComponent to make weapon animation runnable.
        const glm::vec2 playerPosWorld =
            coordinatesTransformer.PhysicsToWorld(physicalBody.bodyRAII->GetBody()->GetPosition());
        float angle = atan2(playerInfo.weaponDirection.y, playerInfo.weaponDirection.x);
        auto weaponAnimation = resourceManager.GetAnimation("automaticWeapon");
        SDL_RendererFlip weaponFlip =
            animationComponent.flip == SDL_FLIP_HORIZONTAL ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;
        primitivesRenderer.RenderAnimationFirstFrame(weaponAnimation, playerPosWorld, angle, weaponFlip);
    }
}

void GameObjectsRenderSystem::RenderAnimations()
{
    auto view = registry.view<AnimationComponent, PhysicsComponent>();

    for (auto entity : view)
    {
        const auto& [animationInfo, physicsInfo] = view.get<AnimationComponent, PhysicsComponent>(entity);

        // Caclulate the position and angle of the animation.
        auto body = physicsInfo.bodyRAII->GetBody();
        glm::vec2 physicsBodyCenterWorld = coordinatesTransformer.PhysicsToWorld(body->GetPosition());
        const float angle = body->GetAngle();

        primitivesRenderer.RenderAnimation(animationInfo, physicsBodyCenterWorld, angle);

        if (utils::GetConfig<bool, "GameObjectsRenderSystem.renderPlayerHitbox">())
        {
            primitivesRenderer.RenderSquare(
                physicsBodyCenterWorld, animationInfo.GetHitboxSize(), ColorName::Green, angle);
        }
    }
}
