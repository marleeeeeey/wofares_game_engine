#include "objects_factory.h"
#include "ecs/components/animation_components.h"
#include <ecs/components/physics_components.h>
#include <ecs/components/player_components.h>
#include <ecs/components/portal_components.h>
#include <ecs/components/rendering_components.h>
#include <ecs/components/timer_components.h>
#include <ecs/components/weapon_components.h>
#include <entt/entity/fwd.hpp>
#include <my_cpp_utils/config.h>
#include <my_cpp_utils/math_utils.h>
#include <unordered_map>
#include <utils/box2d/box2d_body_options.h>
#include <utils/box2d/box2d_body_tuner.h>
#include <utils/box2d/box2d_utils.h>
#include <utils/coordinates_transformer.h>
#include <utils/entt/entt_registry_wrapper.h>
#include <utils/factories/box2d_body_creator.h>
#include <utils/factories/weapon_props_factory.h>
#include <utils/logger.h>
#include <utils/sdl/sdl_texture_process.h>
#include <utils/sdl/sdl_utils.h>

ObjectsFactory::ObjectsFactory(EnttRegistryWrapper& registryWrapper, ResourceManager& resourceManager)
  : registryWrapper(registryWrapper), registry(registryWrapper.GetRegistry()), resourceManager(resourceManager),
    gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())), box2dBodyCreator(registry),
    coordinatesTransformer(registry), bodyTuner(registry)
{}

entt::entity ObjectsFactory::SpawnTile(
    glm::vec2 posWorld, float sizeWorld, const TextureRect& textureRect, SpawnTileOption tileOptions,
    const std::string& name)
{
    auto& gap = utils::GetConfig<float, "ObjectsFactory.gapBetweenPhysicalAndVisual">();
    glm::vec2 bodySizeWorld(sizeWorld - gap, sizeWorld - gap);

    auto entity = registryWrapper.Create(name);
    registry.emplace<RenderingComponent>(
        entity, glm::vec2(sizeWorld, sizeWorld), textureRect.texture, textureRect.rect, tileOptions.zOrderingType);

    Box2dBodyOptions options;
    switch (tileOptions.destructibleOption)
    {
    case SpawnTileOption::DesctructibleOption::Destructible:
        options.anglePolicy = Box2dBodyOptions::AnglePolicy::Dynamic;
        registry.emplace<DestructibleByPlayerComponent>(entity);
        break;
    case SpawnTileOption::DesctructibleOption::NoDestructible:
        options.dynamic = Box2dBodyOptions::MovementPolicy::Manual;
        options.anglePolicy = Box2dBodyOptions::AnglePolicy::Fixed;
        options.collisionPolicy = {CollisionFlags::None, CollisionFlags::None};
        break;
    }
    box2dBodyCreator.CreatePhysicsBody(entity, posWorld, bodySizeWorld, options);

    return entity;
}

entt::entity ObjectsFactory::SpawnPlayer(const glm::vec2& posWorld, const std::string& debugName)
{
    auto entity = registryWrapper.Create(debugName);

    // AnimationInfo.
    AnimationComponent playerAnimation = CreateAnimationInfo("player", "Run", ResourceManager::TagProps::ExactMatch);
    registry.emplace<AnimationComponent>(entity, playerAnimation);

    // PlayerInfo.
    auto& playerInfo = registry.emplace<PlayerComponent>(entity);
    playerInfo.weapons = WeaponPropsFactory::CreateAllWeaponsSet();

    // PhysicsInfo.
    Box2dBodyOptions options;
    options.shape = Box2dBodyOptions::Shape::Capsule;
    options.sensor = Box2dBodyOptions::Sensor::ThinSensorBelow;
    options.dynamic = Box2dBodyOptions::MovementPolicy::Box2dPhysics;
    options.anglePolicy = Box2dBodyOptions::AnglePolicy::Fixed;
    glm::vec2 playerHitboxSizeWorld = playerAnimation.GetHitboxSize();
    box2dBodyCreator.CreatePhysicsBody(entity, posWorld, playerHitboxSizeWorld, options);
    MY_LOG(debug, "Create player body with bbox: {}", playerHitboxSizeWorld);

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
    options.dynamic = Box2dBodyOptions::MovementPolicy::Box2dPhysics;
    options.shape = Box2dBodyOptions::Shape::Box;
    options.anglePolicy = anglePolicy;
    auto& physicsBody = box2dBodyCreator.CreatePhysicsBody(flyingEntity, posWorld, sizeWorld, options);

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
        MY_LOG(warn, "[CreateBullet] Player does not have all of the required components. Entity: {}", playerEntity);
        return entt::null;
    }
    const auto& playerInfo = registry.get<PlayerComponent>(playerEntity);

    // 1. Check if player has weapon set as current.
    if (!playerInfo.weapons.contains(playerInfo.currentWeapon))
    {
        MY_LOG(
            trace, "[CreateBullet] Player does not have {} weapon set as current. Entity: {}", playerInfo.currentWeapon,
            playerEntity);
        return entt::null;
    }
    const WeaponProps& weaponProps = playerInfo.weapons.at(playerInfo.currentWeapon);

    // Prepare the bullet animation.
    AnimationComponent fireballAnimation =
        CreateAnimationInfo(weaponProps.animationName, weaponProps.animationTag, ResourceManager::TagProps::ExactMatch);
    glm::vec2 fireballHitboxSizeWorld = fireballAnimation.GetHitboxSize();

    // Spawn flying entity.
    const auto& playerBody = registry.get<PhysicsComponent>(playerEntity).bodyRAII->GetBody();
    const auto& playerAnimationComponent = registry.get<AnimationComponent>(playerEntity);
    glm::vec2 playerSizeWorld = playerAnimationComponent.GetHitboxSize();
    const auto& weaponDirection = playerInfo.weaponDirection;
    glm::vec2 playerPosWorld = coordinatesTransformer.PhysicsToWorld(playerBody->GetPosition());
    auto weaponInitialPointShift = weaponDirection * (playerSizeWorld.x / 2.0f + fireballHitboxSizeWorld.x);
    glm::vec2 positionInFrontOfPlayer = playerPosWorld + weaponInitialPointShift;
    entt::entity bulletEntity = SpawnFlyingEntity(
        positionInFrontOfPlayer, fireballHitboxSizeWorld, weaponDirection, initialBulletSpeed, anglePolicy);
    bodyTuner.ApplyOption(bulletEntity, Box2dBodyOptions::BulletPolicy::Bullet);
    bodyTuner.ApplyOption(
        bulletEntity, Box2dBodyOptions::CollisionPolicy{CollisionFlags::Bullet, CollisionFlags::Default});

    // Add the bullet animation to the bullet entity.
    registry.emplace<AnimationComponent>(bulletEntity, fireballAnimation);

    // Create damage component.
    DamageComponent damageComponent;
    damageComponent.force = weaponProps.damageForce;
    damageComponent.radius = coordinatesTransformer.WorldToPhysics(weaponProps.damageRadiusWorld);

    switch (playerInfo.currentWeapon)
    {
    case WeaponType::Bazooka:
        registry.emplace<DamageComponent>(bulletEntity, damageComponent);
        registry.emplace<ExplosionOnContactComponent>(bulletEntity);
        break;
    case WeaponType::Grenade:
        registry.emplace<DamageComponent>(bulletEntity, damageComponent);
        registry.emplace<TimerComponent>(bulletEntity, 3.0f);
        registry.emplace<ExplosionOnTimerComponent>(bulletEntity);
        break;
    }

    return bulletEntity;
}

entt::entity ObjectsFactory::SpawnBuildingBlock(glm::vec2 posWorld)
{
    auto entity = registryWrapper.Create("BuildingBlock");
    glm::vec2 sizeWorld(10.0f, 10.0f);
    box2dBodyCreator.CreatePhysicsBody(entity, posWorld, sizeWorld);

    AnimationComponent buildingBlockAnimation =
        CreateAnimationInfo("buildingBlock", "block", ResourceManager::TagProps::ExactMatch);
    registry.emplace<AnimationComponent>(entity, buildingBlockAnimation);

    registry.emplace<DestructibleByPlayerComponent>(entity);

    return entity;
}

entt::entity ObjectsFactory::SpawnPortal(const glm::vec2& posWorld, const std::string& debugName)
{
    auto entity = registryWrapper.Create(debugName);

    // AnimationInfo.
    AnimationComponent portalAnimation =
        CreateAnimationInfo("portal", "Suction", ResourceManager::TagProps::ExactMatch);
    registry.emplace<AnimationComponent>(entity, portalAnimation);

    // PortalComponent.
    registry.emplace<PortalComponent>(entity);

    // PhysicsInfo.
    Box2dBodyOptions options;
    options.shape = Box2dBodyOptions::Shape::Capsule;
    options.dynamic = Box2dBodyOptions::MovementPolicy::Box2dPhysicsNoGravity;
    options.anglePolicy = Box2dBodyOptions::AnglePolicy::Fixed;
    options.collisionPolicy = {CollisionFlags::None, CollisionFlags::None};
    glm::vec2 playerHitboxSizeWorld = portalAnimation.GetHitboxSize();
    box2dBodyCreator.CreatePhysicsBody(entity, posWorld, playerHitboxSizeWorld, options);
    MY_LOG(debug, "Create Portal body with bbox: {}", playerHitboxSizeWorld);

    registry.emplace<TimerComponent>(
        entity, 0,
        [this](entt::entity timedPortal)
        {
            // Update the speed of the portal object randomly.
            auto& portalComponent = registry.get<PortalComponent>(timedPortal);
            portalComponent.speed = utils::Random<float>(0.5, 1.5);

            // Reset the timer.
            auto& timerComponent = registry.get<TimerComponent>(timedPortal);
            timerComponent.timeToActivation = utils::Random<float>(4, 20);
            timerComponent.isActivated = false;

            MY_LOG(debug, "Portal {} changing speed to {}", timedPortal, portalComponent.speed);
        });

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
        if (!registry.all_of<PhysicsComponent, RenderingComponent>(entity))
            continue;

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

            ObjectsFactory::SpawnTileOption spawnTileOptions;
            spawnTileOptions.destructibleOption = ObjectsFactory::SpawnTileOption::DesctructibleOption::Destructible;
            spawnTileOptions.zOrderingType = ZOrderingType::Terrain;

            auto pixelEntity = SpawnTile(
                pixelCenterWorld, cellSizeWorld.x, TextureRect{originalObjRenderingInfo.texturePtr, pixelTextureRect},
                spawnTileOptions, "PixeledTile");

            registry.emplace<PixeledTileComponent>(pixelEntity);

            splittedEntities.push_back(pixelEntity);
        }
    }

    return splittedEntities;
}