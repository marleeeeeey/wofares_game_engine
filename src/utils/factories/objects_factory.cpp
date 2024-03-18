#include "objects_factory.h"
#include "my_common_cpp_utils/config.h"
#include "my_common_cpp_utils/logger.h"
#include "my_common_cpp_utils/math_utils.h"
#include "utils/entt_registry_wrapper.h"
#include "utils/factories/weapon_props_factory.h"
#include "utils/math_utils.h"
#include "utils/physics_methods.h"
#include "utils/sdl_texture_process.h"
#include <ecs/components/game_components.h>
#include <unordered_map>
#include <utils/coordinates_transformer.h>
#include <utils/factories/box2d_body_creator.h>
#include <utils/sdl_utils.h>

ObjectsFactory::ObjectsFactory(EnttRegistryWrapper& registryWrapper, ResourceManager& resourceManager)
  : registryWrapper(registryWrapper), registry(registryWrapper.GetRegistry()), resourceManager(resourceManager),
    gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())), physicsWorld(gameState.physicsWorld),
    box2dBodyCreator(registry), coordinatesTransformer(registry), bodyTuner(registry)
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
    auto entity = registryWrapper.Create("Player");

    // AnimationInfo.
    AnimationInfo playerAnimation = CreateAnimationInfo("player_walk", "Run", ResourceManager::TagProps::ExactMatch);
    registry.emplace<AnimationInfo>(entity, playerAnimation);

    // PlayerInfo.
    auto& playerInfo = registry.emplace<PlayerInfo>(entity);
    playerInfo.weapons = WeaponPropsFactory::CreateAllWeaponsSet();

    // PhysicsInfo.
    Box2dBodyCreator::Options options;
    options.shape = Box2dBodyCreator::Options::Shape::Capsule;
    options.hasSensorBelowTheBody = true;
    options.isDynamic = true;
    auto playerPhysicsBody = box2dBodyCreator.CreatePhysicsBody(entity, sdlPos, playerAnimation.sdlBBox, options);
    MY_LOG_FMT(info, "Create player body with bbox: {}", playerAnimation.sdlBBox);
    registry.emplace<PhysicsInfo>(entity, playerPhysicsBody);

    return entity;
}

entt::entity ObjectsFactory::SpawnFragmentAfterExplosion(const glm::vec2& sdlWorldPos)
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
    const glm::vec2& posWorld, const glm::vec2& sizeWorld, const glm::vec2& forceDirection, float initialSpeed)
{
    // Create the flying entity.
    auto flyingEntity = registryWrapper.Create("flyingEntity");

    // Create a Box2D body for the flying entity.
    Box2dBodyCreator::Options options;
    options.isDynamic = true;
    options.shape = Box2dBodyCreator::Options::Shape::Box;
    auto physicsBody = box2dBodyCreator.CreatePhysicsBody(flyingEntity, posWorld, sizeWorld, options);

    registry.emplace<RenderingInfo>(flyingEntity, sizeWorld);
    registry.emplace<PhysicsInfo>(flyingEntity, physicsBody);

    // Apply the force to the flying entity.
    b2Vec2 speedVec = b2Vec2(initialSpeed, 0);
    speedVec = b2Mul(b2Rot(atan2(forceDirection.y, forceDirection.x)), speedVec);
    physicsBody->GetBody()->SetLinearVelocity(speedVec);

    return flyingEntity;
};

entt::entity ObjectsFactory::CreateBullet(entt::entity playerEntity, float initialBulletSpeed)
{
    if (!registry.all_of<PlayerInfo, PhysicsInfo, AnimationInfo>(playerEntity))
    {
        MY_LOG_FMT(
            warn, "[CreateBullet] Player does not have all of the required components. Entity: {}",
            static_cast<int>(playerEntity));
        return entt::null;
    }
    const auto& playerInfo = registry.get<PlayerInfo>(playerEntity);

    // 1. Check if player has weapon set as current.
    if (!playerInfo.weapons.contains(playerInfo.currentWeapon))
    {
        MY_LOG_FMT(
            trace, "[CreateBullet] Player does not have {} weapon set as current. Entity: {}", playerInfo.currentWeapon,
            static_cast<int>(playerEntity));
        return entt::null;
    }
    const WeaponProps& currentWeaponProps = playerInfo.weapons.at(playerInfo.currentWeapon);

    // 4. Create bullet entity.
    ExplosionImpactComponent explosionImpact;
    explosionImpact.force = currentWeaponProps.damageForce;
    explosionImpact.radius = coordinatesTransformer.WorldToPhysics(currentWeaponProps.damageRadiusWorld);

    // Spawn flying entity.
    const auto& playerBody = registry.get<PhysicsInfo>(playerEntity).bodyRAII->GetBody();
    const auto& animationInfo = registry.get<AnimationInfo>(playerEntity);
    // TODO0: use the size from specific bounding box. Do not like that Animation info impact on physics.
    const auto& playerSize = animationInfo.animation.frames.front().renderingInfo.sdlSize;
    const auto& weaponDirection = playerInfo.weaponDirection;
    glm::vec2 playerWorldPos = coordinatesTransformer.PhysicsToWorld(playerBody->GetPosition());
    glm::vec2 positionInFrontOfPlayer = playerWorldPos + weaponDirection * playerSize.x / 2.0f;
    entt::entity bulletEntity = SpawnFlyingEntity(
        positionInFrontOfPlayer, currentWeaponProps.projectileSizeWorld, weaponDirection, initialBulletSpeed);
    bodyTuner.SetBulletFlagForTheEntity(bulletEntity, true);

    // TODO1: Think about merging all explosion components into one.
    // All weapons setting should be in one place - WeaponProps.
    registry.emplace<ExplosionImpactComponent>(bulletEntity, explosionImpact);

    // Apply the specific explosion component to the bullet entity.
    if (playerInfo.currentWeapon == WeaponType::Grenade)
    {
        registry.emplace<TimerExplosionComponent>(bulletEntity);
    }
    else
    {
        registry.emplace<ContactExplosionComponent>(bulletEntity);
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
        auto fragmentEntity = SpawnFragmentAfterExplosion(fragmentRandomPos);
        fragments.push_back(fragmentEntity);
    }
    PhysicsMethods physicsMethods(registry);
    physicsMethods.ApplyForceToPhysicalBodies(fragments, centerWorld, 500.0f);

    return fragments;
};

std::vector<entt::entity> ObjectsFactory::SpawnSplittedPhysicalEnteties(
    const std::vector<entt::entity>& physicalEntities, SDL_Point cellSize)
{
    assert(cellSize.x == cellSize.y);

    std::vector<entt::entity> splittedEntities;

    for (auto& entity : physicalEntities)
    {
        auto originalObjPhysicsInfo = registry.get<PhysicsInfo>(entity).bodyRAII->GetBody();
        auto& originalObjRenderingInfo = registry.get<RenderingInfo>(entity);
        const b2Vec2& physicsPos = originalObjPhysicsInfo->GetPosition();
        const glm::vec2 originalObjCenterWorld = coordinatesTransformer.PhysicsToWorld(physicsPos);
        const SDL_Rect& originalTextureRect = originalObjRenderingInfo.textureRect;

        // Check if the original object is big enough to be splitted.
        if (originalTextureRect.w <= cellSize.x || originalTextureRect.h <= cellSize.y)
            continue;

        auto originalRectCenterInTexture = utils::GetCenterOfRect(originalTextureRect);

        auto pixelTextureRects = utils::DivideRectByCellSize(originalTextureRect, cellSize);
        for (auto& pixelTextureRect : pixelTextureRects)
        {
            auto pixelRectCenterInTexture = utils::GetCenterOfRect(pixelTextureRect);
            auto pixelCenterShift = pixelRectCenterInTexture - originalRectCenterInTexture;
            glm::vec2 pixelCenterWorld = originalObjCenterWorld + pixelCenterShift;

            auto pixelEntity = CreateTile(
                pixelCenterWorld, cellSize.x, TextureRect{originalObjRenderingInfo.texturePtr, pixelTextureRect},
                "PixeledTile");

            splittedEntities.push_back(pixelEntity);
        }
    }

    return splittedEntities;
};