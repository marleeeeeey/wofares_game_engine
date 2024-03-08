#include "objects_factory.h"
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

    AnimationInfo playerAnimation = resourceManager.GetAnimation("player_walk");
    playerAnimation.isPlaying = true;
    auto& renderingInfo = playerAnimation.frames.front().renderingInfo;
    auto playerSdlSize = renderingInfo.sdlSize;
    glm::vec2 playerSdlBBox = playerSdlSize - glm::vec2{gap, gap};

    auto entity = registryWrapper.Create("Player");
    registry.emplace<AnimationInfo>(entity, playerAnimation);
    registry.emplace<PlayerInfo>(entity);

    // Create a Box2D body for the player.
    Box2dBodyCreator::Options options;
    options.shape = Box2dBodyCreator::Options::Shape::Capsule;
    options.hasSensorBelowTheBody = true;
    options.isDynamic = true;
    auto playerPhysicsBody = box2dBodyCreator.CreatePhysicsBody(entity, sdlPos, playerSdlBBox, options);

    registry.emplace<PhysicsInfo>(entity, playerPhysicsBody);
    return entity;
}
