#include "objects_factory.h"
#include "ecs/components/portal_components.h"
#include "utils/box2d_body_options.h"
#include <ecs/components/physics_components.h>
#include <ecs/components/player_components.h>
#include <ecs/components/rendering_components.h>
#include <ecs/components/weapon_components.h>
#include <my_cpp_utils/config.h>
#include <my_cpp_utils/logger.h>
#include <my_cpp_utils/math_utils.h>
#include <unordered_map>
#include <utils/box2d_utils.h>
#include <utils/coordinates_transformer.h>
#include <utils/entt_registry_wrapper.h>
#include <utils/factories/box2d_body_creator.h>
#include <utils/factories/weapon_props_factory.h>
#include <utils/sdl_texture_process.h>
#include <utils/sdl_utils.h>

ObjectsFactory::ObjectsFactory(EnttRegistryWrapper& registryWrapper, ResourceManager& resourceManager)
  : registryWrapper(registryWrapper), registry(registryWrapper.GetRegistry()), resourceManager(resourceManager),
    gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())), physicsWorld(gameState.physicsWorld),
    box2dBodyCreator(registry), coordinatesTransformer(registry), bodyTuner(registry)
{}

entt::entity ObjectsFactory::SpawnTile(
    glm::vec2 posWorld, float sizeWorld, const TextureRect& textureRect, const std::string& name)
{
    auto& gap = utils::GetConfig<float, "ObjectsFactory.gapBetweenPhysicalAndVisual">();
    glm::vec2 bodySizeWorld(sizeWorld - gap, sizeWorld - gap);

    auto entity = registryWrapper.Create(name);
    registry.emplace<RenderingComponent>(
        entity, glm::vec2(sizeWorld, sizeWorld), textureRect.texture, textureRect.rect);

    // Create a Box2D body for the tile.
    Box2dBodyOptions options;
    options.anglePolicy = Box2dBodyOptions::AnglePolicy::Dynamic;
    box2dBodyCreator.CreatePhysicsBody(entity, posWorld, bodySizeWorld, options);

    return entity;
}

entt::entity ObjectsFactory::SpawnPlayer(const glm::vec2& posWorld, const std::string& debugName)
{
    auto entity = registryWrapper.Create(debugName);

    // AnimationInfo.
    AnimationComponent playerAnimation =
        CreateAnimationInfo("playerWalk", "Run", ResourceManager::TagProps::ExactMatch);
    registry.emplace<AnimationComponent>(entity, playerAnimation);

    // PlayerInfo.
    auto& playerInfo = registry.emplace<PlayerComponent>(entity);
    playerInfo.weapons = WeaponPropsFactory::CreateAllWeaponsSet();

    // PhysicsInfo.
    Box2dBodyOptions options;
    options.shape = Box2dBodyOptions::Shape::Capsule;
    options.sensor = Box2dBodyOptions::Sensor::ThinSensorBelow;
    options.dynamic = Box2dBodyOptions::DynamicOption::Dynamic;
    options.anglePolicy = Box2dBodyOptions::AnglePolicy::Fixed;
    glm::vec2 playerHitboxSizeWorld = playerAnimation.GetHitboxSize();
    box2dBodyCreator.CreatePhysicsBody(entity, posWorld, playerHitboxSizeWorld, options);
    MY_LOG(info, "Create player body with bbox: {}", playerHitboxSizeWorld);

    return entity;
}

entt::entity ObjectsFactory::SpawnFragmentAfterExplosion(const glm::vec2& posWorld)
{
    AnimationComponent fragmentAnimation =
        CreateAnimationInfo("explosionFragments", "Fragment[\\d]+", ResourceManager::TagProps::RandomByRegex);
    glm::vec2 fragmentSizeWorld = fragmentAnimation.GetHitboxSize();

    auto entity = registryWrapper.Create("ExplosionFragment");
    registry.emplace<AnimationComponent>(entity, fragmentAnimation);
    box2dBodyCreator.CreatePhysicsBody(entity, posWorld, fragmentSizeWorld);
    registry.emplace<CollisionDisableHitCountComponent>(
        entity, utils::GetConfig<size_t, "ObjectsFactory.numberOfHitsToDisableCollisionsForFragments">());
    return entity;
}

AnimationComponent ObjectsFactory::CreateAnimationInfo(
    const std::string& animationName, const std::string& tagName, ResourceManager::TagProps tagProps)
{
    AnimationComponent animationInfo;
    animationInfo.animation = resourceManager.GetAnimation(animationName, tagName, tagProps);
    animationInfo.isPlaying = true;
    return animationInfo;
}

entt::entity ObjectsFactory::SpawnFlyingEntity(
    const glm::vec2& posWorld, const glm::vec2& sizeWorld, const glm::vec2& forceDirection, float initialSpeed,
    Box2dBodyOptions::AnglePolicy anglePolicy)
{
    // Create the flying entity.
    auto flyingEntity = registryWrapper.Create("flyingEntity");

    // Create a Box2D body for the flying entity.
    Box2dBodyOptions options;
    options.dynamic = Box2dBodyOptions::DynamicOption::Dynamic;
    options.shape = Box2dBodyOptions::Shape::Box;
    options.anglePolicy = anglePolicy;
    auto& physicsBody = box2dBodyCreator.CreatePhysicsBody(flyingEntity, posWorld, sizeWorld, options);

    registry.emplace<RenderingComponent>(flyingEntity, sizeWorld);

    // Apply the force to the flying entity.
    b2Vec2 speedVec = b2Vec2(initialSpeed, 0);
    speedVec = b2Mul(b2Rot(atan2(forceDirection.y, forceDirection.x)), speedVec);
    physicsBody.bodyRAII->GetBody()->SetLinearVelocity(speedVec);

    return flyingEntity;
}

entt::entity ObjectsFactory::SpawnBullet(
    entt::entity playerEntity, float initialBulletSpeed, Box2dBodyOptions::AnglePolicy anglePolicy)
{
    if (!registry.all_of<PlayerComponent, PhysicsComponent, AnimationComponent>(playerEntity))
    {
        MY_LOG(
            warn, "[CreateBullet] Player does not have all of the required components. Entity: {}",
            static_cast<int>(playerEntity));
        return entt::null;
    }
    const auto& playerInfo = registry.get<PlayerComponent>(playerEntity);

    // 1. Check if player has weapon set as current.
    if (!playerInfo.weapons.contains(playerInfo.currentWeapon))
    {
        MY_LOG(
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
    const auto& playerBody = registry.get<PhysicsComponent>(playerEntity).bodyRAII->GetBody();
    const auto& animationInfo = registry.get<AnimationComponent>(playerEntity);
    glm::vec2 playerSizeWorld = animationInfo.GetHitboxSize();
    const auto& weaponDirection = playerInfo.weaponDirection;
    glm::vec2 playerPosWorld = coordinatesTransformer.PhysicsToWorld(playerBody->GetPosition());
    auto weaponInitialPointShift =
        weaponDirection * (playerSizeWorld.x / 2.0f + currentWeaponProps.projectileSizeWorld.x);
    glm::vec2 positionInFrontOfPlayer = playerPosWorld + weaponInitialPointShift;
    entt::entity bulletEntity = SpawnFlyingEntity(
        positionInFrontOfPlayer, currentWeaponProps.projectileSizeWorld, weaponDirection, initialBulletSpeed,
        anglePolicy);
    bodyTuner.ApplyOption(bulletEntity, Box2dBodyOptions::BulletPolicy::Bullet);

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
}

entt::entity ObjectsFactory::SpawnBuildingBlock(glm::vec2 posWorld)
{
    auto entity = registryWrapper.Create("BuildingBlock");
    glm::vec2 sizeWorld(10.0f, 10.0f);
    box2dBodyCreator.CreatePhysicsBody(entity, posWorld, sizeWorld);
    registry.emplace<RenderingComponent>(entity, sizeWorld, nullptr, SDL_Rect{}, ColorName::Green);
    return entity;
}

entt::entity ObjectsFactory::SpawnPortal(const glm::vec2& posWorld, const std::string& debugName)
{
    auto entity = registryWrapper.Create(debugName);

    // AnimationInfo.
    AnimationComponent playerAnimation =
        CreateAnimationInfo("playerWalk", "Run", ResourceManager::TagProps::ExactMatch);
    registry.emplace<AnimationComponent>(entity, playerAnimation);

    // PortalComponent.
    registry.emplace<PortalComponent>(entity);

    // PhysicsInfo.
    Box2dBodyOptions options;
    options.shape = Box2dBodyOptions::Shape::Capsule;
    options.dynamic = Box2dBodyOptions::DynamicOption::Static;
    options.anglePolicy = Box2dBodyOptions::AnglePolicy::Fixed;
    options.collisionPolicy = Box2dBodyOptions::CollisionPolicy::NoCollision;
    glm::vec2 playerHitboxSizeWorld = playerAnimation.GetHitboxSize();
    box2dBodyCreator.CreatePhysicsBody(entity, posWorld, playerHitboxSizeWorld, options);
    MY_LOG(info, "Create Portal body with bbox: {}", playerHitboxSizeWorld);

    return entity;
}

std::vector<entt::entity> ObjectsFactory::SpawnFragmentsAfterExplosion(glm::vec2 centerWorld, float radiusWorld)
{
    size_t fragmentsCount = static_cast<size_t>(radiusWorld * 0.2f * utils::Random<float>(1, 1.2));
    std::vector<entt::entity> fragments;
    for (size_t i = 0; i < fragmentsCount; ++i)
    {
        auto fragmentRandomPosWorld = utils::GetRandomCoordinateAround(centerWorld, radiusWorld);
        auto fragmentEntity = SpawnFragmentAfterExplosion(fragmentRandomPosWorld);
        fragments.push_back(fragmentEntity);
    }
    Box2dUtils physicsMethods(registry);
    physicsMethods.ApplyForceToPhysicalBodies(fragments, centerWorld, 500.0f);

    return fragments;
}

std::vector<entt::entity> ObjectsFactory::SpawnSplittedPhysicalEnteties(
    const std::vector<entt::entity>& physicalEntities, SDL_Point cellSizeWorld)
{
    assert(cellSizeWorld.x == cellSizeWorld.y);

    std::vector<entt::entity> splittedEntities;

    for (auto& entity : physicalEntities)
    {
        auto originalObjPhysicsInfo = registry.get<PhysicsComponent>(entity).bodyRAII->GetBody();
        auto& originalObjRenderingInfo = registry.get<RenderingComponent>(entity);
        const b2Vec2& posPhysics = originalObjPhysicsInfo->GetPosition();
        const glm::vec2 originalObjCenterWorld = coordinatesTransformer.PhysicsToWorld(posPhysics);
        const SDL_Rect& originalTextureRect = originalObjRenderingInfo.textureRect;

        // Check if the original object is big enough to be splitted.
        if (originalTextureRect.w <= cellSizeWorld.x || originalTextureRect.h <= cellSizeWorld.y)
            continue;

        auto originalRectCenterInTexture = utils::GetCenterOfRect(originalTextureRect);

        auto pixelTextureRects = utils::DivideRectByCellSize(originalTextureRect, cellSizeWorld);
        for (auto& pixelTextureRect : pixelTextureRects)
        {
            auto pixelRectCenterInTexture = utils::GetCenterOfRect(pixelTextureRect);
            auto pixelCenterShift = pixelRectCenterInTexture - originalRectCenterInTexture;
            glm::vec2 pixelCenterWorld = originalObjCenterWorld + pixelCenterShift;

            auto pixelEntity = SpawnTile(
                pixelCenterWorld, cellSizeWorld.x, TextureRect{originalObjRenderingInfo.texturePtr, pixelTextureRect},
                "PixeledTile");

            splittedEntities.push_back(pixelEntity);
        }
    }

    return splittedEntities;
}