#include "objects_factory.h"
#include <ecs/components/animation_components.h>
#include <ecs/components/event_components.h>
#include <ecs/components/physics_components.h>
#include <ecs/components/player_components.h>
#include <ecs/components/portal_components.h>
#include <ecs/components/rendering_components.h>
#include <ecs/components/weapon_components.h>
#include <entt/entity/entity.hpp>
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
#include <utils/time_utils.h>

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
    registry.emplace<TileComponent>(
        entity, glm::vec2(sizeWorld, sizeWorld), textureRect.texture, textureRect.rect, tileOptions.zOrderingType);

    Box2dBodyOptions options;
    options.fixture.restitution = 0.05f;
    switch (tileOptions.destructibleOption)
    {
    case SpawnTileOption::DesctructibleOption::Destructible:
        options.anglePolicy = Box2dBodyOptions::AnglePolicy::Dynamic;
        registry.emplace<DestructibleComponent>(entity);
        break;
    case SpawnTileOption::DesctructibleOption::Indestructible:
        options.dynamic = Box2dBodyOptions::MovementPolicy::Manual;
        options.anglePolicy = Box2dBodyOptions::AnglePolicy::Fixed;
        registry.emplace<IndestructibleComponent>(entity);
        break;
    }

    switch (tileOptions.collidableOption)
    {
    case SpawnTileOption::CollidableOption::Collidable:
        // Default collision flags for options.collisionPolicy
        registry.emplace<CollidableComponent>(entity);
        break;
    case SpawnTileOption::CollidableOption::Transparent:
        options.collisionPolicy = {CollisionFlags::None, CollisionFlags::None};
        registry.emplace<TransparentComponent>(entity);
        break;
    }

    float angle = 0.0f;
    box2dBodyCreator.CreatePhysicsBody(entity, posWorld, bodySizeWorld, angle, options);

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
    options.fixture.restitution = 0.05f;
    options.shape = Box2dBodyOptions::Shape::Capsule;
    options.sensor = Box2dBodyOptions::Sensor::ThinSensorBelow;
    options.dynamic = Box2dBodyOptions::MovementPolicy::Box2dPhysics;
    options.anglePolicy = Box2dBodyOptions::AnglePolicy::Fixed;
    options.collisionPolicy.collideWith = CollisionFlags::Default; // No collision with bullets.
    glm::vec2 playerHitboxSizeWorld = playerAnimation.GetHitboxSize();
    float angle = 0.0f;
    box2dBodyCreator.CreatePhysicsBody(entity, posWorld, playerHitboxSizeWorld, angle, options);
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
    float angle = utils::Random<float>(0, 2 * M_PI);
    box2dBodyCreator.CreatePhysicsBody(entity, posWorld, fragmentSizeWorld, angle);
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

    // Calc angle from force direction.
    float angle = atan2(forceDirection.y, forceDirection.x);
    auto& physicsBody = box2dBodyCreator.CreatePhysicsBody(flyingEntity, posWorld, sizeWorld, angle, options);

    // Apply the force to the flying entity.
    b2Vec2 speedVec = b2Vec2(initialSpeed, 0);
    speedVec = b2Mul(b2Rot(angle), speedVec);
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
    auto weaponInitialPointShift = weaponDirection * (playerSizeWorld.x) / 2.0f;
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
        registry.emplace<TimeEventComponent>(bulletEntity, 3.0f);
        registry.emplace<ExplosionOnTimerComponent>(bulletEntity);
        break;
    }

    if (utils::GetConfig<bool, "ObjectsFactory.debugTraceBulletPath">())
        registry.emplace<MarkForTrailDebugComponent>(bulletEntity, 30);

    return bulletEntity;
}

entt::entity ObjectsFactory::SpawnBuildingBlock(glm::vec2 posWorld)
{
    auto entity = registryWrapper.Create("BuildingBlock");
    glm::vec2 sizeWorld(10.0f, 10.0f);
    float angle = 0.0f;
    box2dBodyCreator.CreatePhysicsBody(entity, posWorld, sizeWorld, angle);

    AnimationComponent buildingBlockAnimation =
        CreateAnimationInfo("buildingBlock", "block", ResourceManager::TagProps::ExactMatch);
    registry.emplace<AnimationComponent>(entity, buildingBlockAnimation);

    registry.emplace<DestructibleComponent>(entity);

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
    float angle = 0.0f;
    box2dBodyCreator.CreatePhysicsBody(entity, posWorld, playerHitboxSizeWorld, angle, options);
    MY_LOG(debug, "Create Portal body with bbox: {}", playerHitboxSizeWorld);

    registry.emplace<TimeEventComponent>(
        entity, 0,
        [this](entt::entity timedPortal)
        {
            // Update the speed of the portal object randomly.
            auto& portalComponent = registry.get<PortalComponent>(timedPortal);
            portalComponent.speed = utils::Random<float>(0.5, 1.5);

            // Reset the timer.
            auto& timerComponent = registry.get<TimeEventComponent>(timedPortal);
            timerComponent.timeToActivation = utils::Random<float>(4, 20);
            timerComponent.isActivated = false;

            MY_LOG(debug, "Portal {} changing speed to {}", timedPortal, portalComponent.speed);
        });

    return entity;
}

entt::entity ObjectsFactory::SpawnDebugVisualObject(
    const glm::vec2& posWorld, const glm::vec2& sizeWorld, float angle, const std::string& nameAsKey,
    const DebugSpawnOptions& debugSpawnOptions)
{
    // Search for entity with the same name component.
    auto namedComponents = registry.view<NameComponent, CreationTimeComponent>();
    std::vector<entt::entity> sameNameEntities = {namedComponents.begin(), namedComponents.end()};
    sameNameEntities.erase(
        std::remove_if(
            sameNameEntities.begin(), sameNameEntities.end(),
            [nameAsKey, this](entt::entity e) { return registry.get<NameComponent>(e).name != nameAsKey; }),
        sameNameEntities.end());

    // Check if the entity with the same name already exists.
    if (debugSpawnOptions.spawnPolicy == ObjectsFactory::SpawnPolicyBase::This && sameNameEntities.size() > 0)
    {
        MY_LOG(
            info, "[SpawnDebugVisualObject] Entity with the same name already exists. Entity: {}, SpawnPolicy: {}",
            nameAsKey, debugSpawnOptions.spawnPolicy);
        return entt::null;
    }

    // Destroy some entities with the same name if the limit is reached.
    if (debugSpawnOptions.spawnPolicy != ObjectsFactory::SpawnPolicyBase::All)
    {
        // Sort components by creation time.
        std::sort(
            sameNameEntities.begin(), sameNameEntities.end(),
            [this](entt::entity a, entt::entity b) {
                return registry.get<CreationTimeComponent>(a).creationTime >
                    registry.get<CreationTimeComponent>(b).creationTime;
            });

        auto trailSize = debugSpawnOptions.trailSize;

        if (debugSpawnOptions.spawnPolicy == ObjectsFactory::SpawnPolicyBase::First)
        {
            // Not nessesary to create new entity. Limit is reached.
            if (sameNameEntities.size() > trailSize)
                return entt::null;

            std::reverse(sameNameEntities.begin(), sameNameEntities.end());
        }

        if (sameNameEntities.size() > trailSize)
        {
            for (size_t i = trailSize; i < sameNameEntities.size(); ++i)
            {
                auto itemToRemove = sameNameEntities[i];
                auto creationTime = registry.get<CreationTimeComponent>(itemToRemove).creationTime;
                MY_LOG(
                    trace,
                    "[SpawnDebugVisualObject] Destroying entity with the same name. Entity: {}, CreationTime: {}",
                    nameAsKey, creationTime);
                registryWrapper.Destroy(itemToRemove);
            }
            sameNameEntities.resize(trailSize);
        }
    }

    auto entity = registryWrapper.Create(nameAsKey);
    registry.emplace<DebugVisualObjectComponent>(entity);
    registry.emplace<NameComponent>(entity, nameAsKey);
    registry.emplace<CreationTimeComponent>(entity, utils::CurrentTimeMilliseconds());

    // PhysicsInfo.
    Box2dBodyOptions options;
    options.shape = Box2dBodyOptions::Shape::Box;
    options.dynamic = Box2dBodyOptions::MovementPolicy::Manual;
    options.anglePolicy = Box2dBodyOptions::AnglePolicy::Fixed;
    options.collisionPolicy = {CollisionFlags::None, CollisionFlags::None};
    box2dBodyCreator.CreatePhysicsBody(entity, posWorld, sizeWorld, angle, options);

    MY_LOG(
        trace, "[SpawnDebugVisualObject] Created new DebugVisualObjectComponent: Name {}, Policy {}, Size {}",
        nameAsKey, debugSpawnOptions.spawnPolicy, sameNameEntities.size());
    return entity;
}

entt::entity ObjectsFactory::SpawnDebugVisualObject(
    entt::entity entity, const std::string& nameAsKey, const DebugSpawnOptions& debugSpawnOptions)
{
    auto& physicsComponent = registry.get<PhysicsComponent>(entity);
    auto body = physicsComponent.bodyRAII->GetBody();
    auto bodyPosPhysics = body->GetPosition();
    auto bodyAnglePhysics = body->GetAngle();
    auto sizeWorld = physicsComponent.options.hitbox.sizeWorld;

    auto posWorld = coordinatesTransformer.PhysicsToWorld(bodyPosPhysics);
    auto newEntity = SpawnDebugVisualObject(posWorld, sizeWorld, bodyAnglePhysics, nameAsKey, debugSpawnOptions);
    return newEntity;
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
        if (!registry.all_of<PhysicsComponent, TileComponent>(entity))
            continue;

        auto originalObjPhysicsInfo = registry.get<PhysicsComponent>(entity).bodyRAII->GetBody();
        auto& originalObjRenderingInfo = registry.get<TileComponent>(entity);
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