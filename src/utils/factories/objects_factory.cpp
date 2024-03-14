#include "objects_factory.h"
#include "my_common_cpp_utils/config.h"
#include "my_common_cpp_utils/logger.h"
#include "my_common_cpp_utils/math_utils.h"
#include "utils/entt_registry_wrapper.h"
#include "utils/math_utils.h"
#include "utils/physics_methods.h"
#include "utils/sdl_texture_process.h"
#include <ecs/components/game_components.h>
#include <utils/coordinates_transformer.h>
#include <utils/factories/box2d_body_creator.h>

ObjectsFactory::ObjectsFactory(EnttRegistryWrapper& registryWrapper, ResourceManager& resourceManager)
  : registryWrapper(registryWrapper), registry(registryWrapper.GetRegistry()), resourceManager(resourceManager),
    gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())), physicsWorld(gameState.physicsWorld),
    box2dBodyCreator(registry), transformer(registry)
{}

entt::entity ObjectsFactory::CreateTile(
    glm::vec2 posWorld, float sizeWorld, const TextureRect& textureRect, const std::string& name)
{
    auto& gap = utils::GetConfig<float, "ObjectsFactory.gapBetweenPhysicalAndVisual">();
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
    registry.emplace<CollisionDisableHitCountComponent>(
        entity, utils::GetConfig<size_t, "ObjectsFactory.numberOfHitsToDisableCollisionsForFragments">());
    return entity;
};

AnimationInfo ObjectsFactory::CreateAnimationInfo(
    const std::string& animationName, const std::string& tagName, ResourceManager::TagProps tagProps)
{
    auto& gap = utils::GetConfig<float, "ObjectsFactory.gapBetweenPhysicalAndVisual">();

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

entt::entity ObjectsFactory::SpawnFlyingEntity(
    const glm::vec2& posWorld, const glm::vec2& sizeWorld, const glm::vec2& forceDirection, float force)
{
    // Create the flying entity.
    auto flyingEntity = registryWrapper.Create("flyingEntity");

    // Create a Box2D body for the flying entity.
    Box2dBodyCreator::Options options;
    options.isDynamic = true;
    auto physicsBody = box2dBodyCreator.CreatePhysicsBody(flyingEntity, posWorld, sizeWorld, options);

    registry.emplace<RenderingInfo>(flyingEntity, sizeWorld);
    registry.emplace<PhysicsInfo>(flyingEntity, physicsBody);

    // Apply the force to the flying entity.
    b2Vec2 forceVec = b2Vec2(force, 0);
    forceVec = b2Mul(b2Rot(atan2(forceDirection.y, forceDirection.x)), forceVec);
    physicsBody->GetBody()->ApplyLinearImpulseToCenter(forceVec, true);

    return flyingEntity;
};

entt::entity ObjectsFactory::CreateBullet(entt::entity entity, float force)
{
    if (!registry.all_of<PlayerInfo, PhysicsInfo, AnimationInfo>(entity))
    {
        MY_LOG_FMT(
            warn, "[CreateBullet] entity does not have all of the required components. Entity: {}",
            static_cast<int>(entity));
        return entt::null;
    }

    const auto& playerInfo = registry.get<PlayerInfo>(entity);
    const auto& playerBody = registry.get<PhysicsInfo>(entity).bodyRAII->GetBody();
    const auto& animationInfo = registry.get<AnimationInfo>(entity);
    // TODO2: use the size from specific bounding box.
    const auto& playerSize = animationInfo.animation.frames.front().renderingInfo.sdlSize;
    const auto& weaponDirection = playerInfo.weaponDirection;

    // Calculate the position of the grenade slightly in front of the player.
    glm::vec2 playerWorldPos = transformer.PhysicsToWorld(playerBody->GetPosition());
    glm::vec2 positionInFrontOfPlayer = playerWorldPos + weaponDirection * playerSize.x;
    glm::vec2 projectileSize(5, 5);

    // Spawn flying entity.
    auto bulletEntity = SpawnFlyingEntity(positionInFrontOfPlayer, projectileSize, weaponDirection, force);

    // Apply the explosion component to the flying entity.
    if (playerInfo.currentWeapon == PlayerInfo::Weapon::Bazooka)
    {
        registry.emplace<ContactExplosionComponent>(bulletEntity);
        registry.emplace<ExplosionImpactComponent>(bulletEntity);
    }
    else if (playerInfo.currentWeapon == PlayerInfo::Weapon::Grenade)
    {
        registry.emplace<TimerExplosionComponent>(bulletEntity);
        registry.emplace<ExplosionImpactComponent>(bulletEntity);
    }

    return bulletEntity;
};

std::vector<entt::entity> ObjectsFactory::SpawnFragmentsAfterExplosion(glm::vec2 centerWorld, float radiusWorld)
{
    size_t fragmentsCount = static_cast<size_t>(radiusWorld * 0.2f * utils::Random<float>(1, 1.2));
    std::vector<entt::entity> fragments;
    for (size_t i = 0; i < fragmentsCount; ++i)
    {
        auto fragmentRandomPos = utils::GetRandomCoordinateAround(centerWorld, radiusWorld);
        auto fragmentEntity = CreateFragmentAfterExplosion(fragmentRandomPos);
        fragments.push_back(fragmentEntity);
    }
    PhysicsMethods physicsMethods(registry);
    physicsMethods.ApplyForceToPhysicalBodies(fragments, centerWorld, 500.0f);

    return fragments;
};

std::vector<entt::entity> ObjectsFactory::SpawnSplittedPhysicalEnteties(
    const std::vector<entt::entity>& physicalEntities, SDL_Point cellSize)
{
    auto physicsWorld = gameState.physicsWorld;
    Box2dBodyCreator box2dBodyCreator(registry);
    CoordinatesTransformer coordinatesTransformer(registry);
    glm::vec2 cellSizeGlm(cellSize.x, cellSize.y);

    std::vector<entt::entity> splittedEntities;

    for (auto& entity : physicalEntities)
    {
        auto originalObjPhysicsInfo = registry.get<PhysicsInfo>(entity).bodyRAII->GetBody();
        auto& originalObjRenderingInfo = registry.get<RenderingInfo>(entity);
        const b2Vec2& physicsPos = originalObjPhysicsInfo->GetPosition();
        const glm::vec2 originalObjWorldPos = coordinatesTransformer.PhysicsToWorld(physicsPos);

        // Check if the original object is big enough to be splitted.
        if (originalObjRenderingInfo.textureRect.w <= cellSize.x ||
            originalObjRenderingInfo.textureRect.h <= cellSize.y)
            continue;

        auto originalRectPosInTexture =
            glm::vec2(originalObjRenderingInfo.textureRect.x, originalObjRenderingInfo.textureRect.y);

        auto textureRects = utils::DivideRectByCellSize(originalObjRenderingInfo.textureRect, cellSize);
        for (auto& rect : textureRects)
        {
            // Caclulate position of the pixel in the world.
            auto pixelRectPosInTexture = glm::vec2(rect.x, rect.y);
            glm::vec2 pixelWorldPosition = originalObjWorldPos + (pixelRectPosInTexture - originalRectPosInTexture) -
                cellSizeGlm - glm::vec2{1, 1}; // TODO1: here is a hack with {1, 1}.

            assert(cellSize.x == cellSize.y);
            auto pixelEntity = CreateTile(
                pixelWorldPosition, cellSize.x, TextureRect{originalObjRenderingInfo.texturePtr, rect}, "PixeledTile");

            splittedEntities.push_back(pixelEntity);
        }
    }

    return splittedEntities;
};