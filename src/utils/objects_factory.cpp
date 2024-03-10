#include "objects_factory.h"
#include "my_common_cpp_utils/logger.h"
#include "utils/entt_registry_wrapper.h"
#include <ecs/components/game_components.h>
#include <utils/box2d_body_creator.h>
#include <utils/coordinates_transformer.h>

ObjectsFactory::ObjectsFactory(EnttRegistryWrapper& registryWrapper, ResourceManager& resourceManager)
  : registryWrapper(registryWrapper), registry(registryWrapper.GetRegistry()), resourceManager(resourceManager),
    gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())), physicsWorld(gameState.physicsWorld),
    box2dBodyCreator(registry)
{}

entt::entity ObjectsFactory::createPlayer(const glm::vec2& sdlPos)
{
    auto gap = gameState.physicsOptions.gapBetweenPhysicalAndVisual;

    AnimationInfo playerAnimation;
    playerAnimation.animation = resourceManager.GetAnimation("player_walk", "Run");
    playerAnimation.isPlaying = true;
    auto& renderingInfo = playerAnimation.animation.frames.front().renderingInfo;

    // Calculate the player's hitbox size.
    glm::vec2 playerSdlBBox;
    if (playerAnimation.animation.hitboxRect)
    {
        playerSdlBBox = glm::vec2(playerAnimation.animation.hitboxRect->w, playerAnimation.animation.hitboxRect->h);
        MY_LOG_FMT(info, "Player hitbox found: {}", playerSdlBBox);
    }
    else
    {
        auto playerSdlSize = renderingInfo.sdlSize;
        playerSdlBBox = playerSdlSize - glm::vec2{gap, gap};
        MY_LOG_FMT(info, "Player hitbox not found, using default hitbox size: {}", playerSdlBBox);
    }

    auto entity = registryWrapper.Create("Player");
    registry.emplace<AnimationInfo>(entity, playerAnimation);
    registry.emplace<PlayerInfo>(entity);

    // Create a Box2D body for the player.
    Box2dBodyCreator::Options options;
    options.shape = Box2dBodyCreator::Options::Shape::Box; // TODO0: here is a bug with capsule bbox calculation.
    options.hasSensorBelowTheBody = true;
    options.isDynamic = true;
    auto playerPhysicsBody = box2dBodyCreator.CreatePhysicsBody(entity, sdlPos, playerSdlBBox, options);
    MY_LOG_FMT(info, "Create player body with bbox: {}", playerSdlBBox);

    registry.emplace<PhysicsInfo>(entity, playerPhysicsBody);
    return entity;
}
