#include "objects_factory.h"
#include "my_common_cpp_utils/logger.h"
#include "my_common_cpp_utils/math_utils.h"
#include "utils/entt_registry_wrapper.h"
#include "utils/texture_process.h"
#include <ecs/components/game_components.h>
#include <utils/box2d_body_creator.h>
#include <utils/coordinates_transformer.h>

ObjectsFactory::ObjectsFactory(EnttRegistryWrapper& registryWrapper, ResourceManager& resourceManager)
  : registryWrapper(registryWrapper), registry(registryWrapper.GetRegistry()), resourceManager(resourceManager),
    gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())), physicsWorld(gameState.physicsWorld),
    box2dBodyCreator(registry)
{}

entt::entity ObjectsFactory::CreateTile(
    glm::vec2 posWorld, float sizeWorld, const TextureRect& textureRect, const std::string& name)
{
    auto gap = gameState.physicsOptions.gapBetweenPhysicalAndVisual;
    glm::vec2 bodySizeWorld(sizeWorld - gap, sizeWorld - gap);

    auto entity = registryWrapper.Create(name);
    registry.emplace<RenderingInfo>(entity, glm::vec2(sizeWorld, sizeWorld), textureRect.texture, textureRect.rect);

    // Create a Box2D body for the tile.
    auto tilePhysicsBody = box2dBodyCreator.CreatePhysicsBody(entity, posWorld, bodySizeWorld);

    // Apply randomly: static/dynamic body.
    tilePhysicsBody->GetBody()->SetType(
        utils::RandomTrue(gameState.levelOptions.dynamicBodyProbability) ? b2_dynamicBody : b2_staticBody);

    registry.emplace<PhysicsInfo>(entity, tilePhysicsBody);

    return entity;
};

entt::entity ObjectsFactory::CreatePlayer(const glm::vec2& sdlPos)
{
    AnimationInfo playerAnimation = CreateAnimationInfo("player_walk", "Run", ResourceManager::TagProps::ExactMatch);

    auto entity = registryWrapper.Create("Player");
    registry.emplace<AnimationInfo>(entity, playerAnimation);
    registry.emplace<PlayerInfo>(entity);

    // Create a Box2D body for the player.
    Box2dBodyCreator::Options options;
    options.shape = Box2dBodyCreator::Options::Shape::Capsule;
    options.hasSensorBelowTheBody = true;
    options.isDynamic = true;
    auto playerPhysicsBody = box2dBodyCreator.CreatePhysicsBody(entity, sdlPos, playerAnimation.sdlBBox, options);
    MY_LOG_FMT(info, "Create player body with bbox: {}", playerAnimation.sdlBBox);

    registry.emplace<PhysicsInfo>(entity, playerPhysicsBody);
    return entity;
}

entt::entity ObjectsFactory::CreateFragmentAfterExplosion(const glm::vec2& sdlWorldPos)
{
    AnimationInfo fragmentAnimation =
        CreateAnimationInfo("explosionFragments", "Fragment[\\d]+", ResourceManager::TagProps::RandomByRegex);

    auto entity = registryWrapper.Create("ExplosionFragment");
    registry.emplace<AnimationInfo>(entity, fragmentAnimation);
    registry.emplace<PhysicsInfo>(
        entity, box2dBodyCreator.CreatePhysicsBody(entity, sdlWorldPos, fragmentAnimation.sdlBBox));
    registry.emplace<CollisionDisableTimerComponent>(entity); // TODO0: replace it to CollisionDisableHitCountComponent
    return entity;
};

AnimationInfo ObjectsFactory::CreateAnimationInfo(
    const std::string& animationName, const std::string& tagName, ResourceManager::TagProps tagProps)
{
    auto gap = gameState.physicsOptions.gapBetweenPhysicalAndVisual;

    AnimationInfo animationInfo;
    animationInfo.animation = resourceManager.GetAnimation(animationName, tagName, tagProps);
    animationInfo.isPlaying = true;
    auto& renderingInfo = animationInfo.animation.frames.front().renderingInfo;

    if (animationInfo.animation.hitboxRect)
    {
        animationInfo.sdlBBox = glm::vec2(animationInfo.animation.hitboxRect->w, animationInfo.animation.hitboxRect->h);
    }
    else
    {
        auto playerSdlSize = renderingInfo.sdlSize;
        animationInfo.sdlBBox = playerSdlSize - glm::vec2{gap, gap};
    }

    return animationInfo;
};
