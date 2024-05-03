#include "objects_factory.h"
#include "ecs/components/turret_component.h"
#include <ecs/components/animation_components.h>
#include <ecs/components/event_components.h>
#include <ecs/components/physics_components.h>
#include <ecs/components/player_components.h>
#include <ecs/components/portal_components.h>
#include <ecs/components/rendering_components.h>
#include <ecs/components/turret_component.h>
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

ObjectsFactory::ObjectsFactory(
    EnttRegistryWrapper& registryWrapper, ComponentsFactory& componentsFactory, BaseObjectsFactory& baseObjectsFactory)
  : registryWrapper(registryWrapper), registry(registryWrapper.GetRegistry()),
    gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())), box2dBodyCreator(registry),
    coordinatesTransformer(registry), bodyTuner(registry), componentsFactory(componentsFactory),
    baseObjectsFactory(baseObjectsFactory)
{}

entt::entity ObjectsFactory::SpawnPlayer(const glm::vec2& posWorld, const std::string& debugName)
{
    auto entity = registryWrapper.Create(debugName);

    // AnimationInfo.
    AnimationComponent playerAnimation =
        componentsFactory.CreateAnimationComponent("player", "Run", ResourceManager::TagProps::ExactMatch);
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
    AnimationComponent fireballAnimation = componentsFactory.CreateAnimationComponent(
        weaponProps.animationName, weaponProps.animationTag, ResourceManager::TagProps::ExactMatch);
    glm::vec2 fireballHitboxSizeWorld = fireballAnimation.GetHitboxSize();

    // Spawn flying entity.
    const auto& playerBody = registry.get<PhysicsComponent>(playerEntity).bodyRAII->GetBody();
    const auto& playerAnimationComponent = registry.get<AnimationComponent>(playerEntity);
    glm::vec2 playerSizeWorld = playerAnimationComponent.GetHitboxSize();
    const auto& weaponDirection = playerInfo.weaponDirection;
    glm::vec2 playerPosWorld = coordinatesTransformer.PhysicsToWorld(playerBody->GetPosition());
    auto weaponInitialPointShift = weaponDirection * (playerSizeWorld.x) / 2.0f;
    glm::vec2 positionInFrontOfPlayer = playerPosWorld + weaponInitialPointShift;
    entt::entity bulletEntity = baseObjectsFactory.SpawnFlyingEntity(
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

    AnimationComponent buildingBlockAnimation =
        componentsFactory.CreateAnimationComponent("buildingBlock", "block", ResourceManager::TagProps::ExactMatch);
    registry.emplace<AnimationComponent>(entity, buildingBlockAnimation);

    float angle = 0.0f;
    box2dBodyCreator.CreatePhysicsBody(entity, posWorld, buildingBlockAnimation.GetHitboxSize(), angle);

    registry.emplace<DestructibleComponent>(entity);

    return entity;
}

entt::entity ObjectsFactory::SpawnPortal(const glm::vec2& posWorld, const std::string& debugName)
{
    auto entity = registryWrapper.Create(debugName);

    // AnimationInfo.
    AnimationComponent portalAnimation =
        componentsFactory.CreateAnimationComponent("portal", "Suction", ResourceManager::TagProps::ExactMatch);
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

entt::entity ObjectsFactory::SpawnTurret(const glm::vec2& posWorld, const std::string& debugName)
{
    auto entity = registryWrapper.Create(debugName);

    // AnimationInfo.
    AnimationComponent portalAnimation =
        componentsFactory.CreateAnimationComponent("turret", "Idle", ResourceManager::TagProps::ExactMatch);
    registry.emplace<AnimationComponent>(entity, portalAnimation);

    // TurretComponent.
    registry.emplace<TurretComponent>(entity);

    // PhysicsInfo.
    Box2dBodyOptions options;
    options.shape = Box2dBodyOptions::Shape::Box;
    options.dynamic = Box2dBodyOptions::MovementPolicy::Box2dPhysics;
    options.anglePolicy = Box2dBodyOptions::AnglePolicy::Dynamic;
    glm::vec2 playerHitboxSizeWorld = portalAnimation.GetHitboxSize();
    float angle = 0.0f;
    box2dBodyCreator.CreatePhysicsBody(entity, posWorld, playerHitboxSizeWorld, angle, options);

    return entity;
}
