#include "player_control_systems.h"
#include "entt/entity/fwd.hpp"
#include "glm/fwd.hpp"
#include "my_common_cpp_utils/config.h"
#include "utils/entt_registry_wrapper.h"
#include <SDL.h>
#include <ecs/components/game_components.h>
#include <imgui_impl_sdl2.h>
#include <my_common_cpp_utils/logger.h>
#include <unordered_map>
#include <utils/coordinates_transformer.h>
#include <utils/factories/box2d_body_creator.h>
#include <utils/systems/input_event_manager.h>

PlayerControlSystem::PlayerControlSystem(
    EnttRegistryWrapper& registryWrapper, InputEventManager& inputEventManager,
    Box2dEnttContactListener& contactListener, ObjectsFactory& objectsFactory)
  : registryWrapper(registryWrapper), registry(registryWrapper.GetRegistry()), inputEventManager(inputEventManager),
    gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())), coordinatesTransformer(registry),
    box2dBodyCreator(registry), contactListener(contactListener), objectsFactory(objectsFactory)
{
    SubscribeToInputEvents();
    SubscribeToContactListener();
}

void PlayerControlSystem::Update(float deltaTime)
{
    const auto& players = registry.view<PlayerInfo>();
    for (auto entity : players)
    {
        UpdateFireRateAndReloadTime(entity, deltaTime);
    }
};

void PlayerControlSystem::SubscribeToInputEvents()
{
    inputEventManager.Subscribe(
        InputEventManager::EventType::ButtonHold,
        [this](const InputEventManager::EventInfo& eventInfo) { HandlePlayerMovement(eventInfo); });

    inputEventManager.Subscribe(
        InputEventManager::EventType::ButtonHold,
        [this](const InputEventManager::EventInfo& eventInfo) { HandlePlayerChangeWeapon(eventInfo); });

    inputEventManager.Subscribe(
        InputEventManager::EventType::ButtonReleaseAfterHold,
        [this](const InputEventManager::EventInfo& eventInfo) { HandlePlayerAttackOnReleaseButton(eventInfo); });

    inputEventManager.Subscribe(
        InputEventManager::EventType::ButtonHold,
        [this](const InputEventManager::EventInfo& eventInfo) { HandlePlayerAttackOnHoldButton(eventInfo); });

    inputEventManager.Subscribe(
        InputEventManager::EventType::RawSdlEvent,
        [this](const InputEventManager::EventInfo& eventInfo) { HandlePlayerBuildingAction(eventInfo); });

    inputEventManager.Subscribe(
        InputEventManager::EventType::RawSdlEvent,
        [this](const InputEventManager::EventInfo& eventInfo) { HandlePlayerWeaponDirection(eventInfo); });
};

void PlayerControlSystem::SubscribeToContactListener()
{
    contactListener.SubscribeContact(
        Box2dEnttContactListener::ContactType::BeginSensor,
        [this](entt::entity entityA, entt::entity entityB) { HandlePlayerBeginPlayerContact(entityA, entityB); });
    contactListener.SubscribeContact(
        Box2dEnttContactListener::ContactType::EndSensor,
        [this](entt::entity entityA, entt::entity entityB) { HandlePlayerEndPlayerContact(entityA, entityB); });
};

void PlayerControlSystem::HandlePlayerMovement(const InputEventManager::EventInfo& eventInfo)
{
    float movingForce = 10.0f;
    float jumpForce = 50.0f;

    auto& originalEvent = eventInfo.originalEvent;

    const auto& players = registry.view<PlayerInfo, PhysicsInfo>();
    for (auto entity : players)
    {
        const auto& [playerInfo, physicalBody] = players.get<PlayerInfo, PhysicsInfo>(entity);
        auto body = physicalBody.bodyRAII->GetBody();

        // If the player is not on the ground, then don't allow to move or jump.
        bool playerControlsWorksOnGroundOnly =
            utils::GetConfig<bool, "PlayerControlSystem.playerControlsWorksOnGroundOnly">();
        if (playerControlsWorksOnGroundOnly && playerInfo.countOfGroundContacts <= 0)
            continue;

        auto mass = body->GetMass();
        movingForce *= mass;
        jumpForce *= mass;

        if (originalEvent.key.keysym.scancode == SDL_SCANCODE_W)
        {
            if (playerInfo.countOfGroundContacts > 0)
                body->ApplyForceToCenter(b2Vec2(0, -jumpForce), true);
        }

        if (originalEvent.key.keysym.scancode == SDL_SCANCODE_A)
        {
            body->ApplyForceToCenter(b2Vec2(-movingForce, 0), true);
        }

        if (originalEvent.key.keysym.scancode == SDL_SCANCODE_D)
        {
            body->ApplyForceToCenter(b2Vec2(movingForce, 0), true);
        }
    }
}

void PlayerControlSystem::HandlePlayerAttackOnReleaseButton(const InputEventManager::EventInfo& eventInfo)
{
    if (eventInfo.originalEvent.button.button == SDL_BUTTON_LEFT)
    {
        const auto& players = registry.view<PlayerInfo, PhysicsInfo, AnimationInfo>();
        for (auto entity : players)
        {
            // TODO2: Make in no linear way.
            float throwingForce = std::min(eventInfo.holdDuration * 0.3f, 0.2f);
            MY_LOG_FMT(debug, "Throwing force: {}", throwingForce);
            MakeShotIfPossible(entity, throwingForce);
        }
    }
}

void PlayerControlSystem::HandlePlayerAttackOnHoldButton(const InputEventManager::EventInfo& eventInfo)
{
    if (eventInfo.originalEvent.button.button == SDL_BUTTON_LEFT)
    {
        const auto& players = registry.view<PlayerInfo, PhysicsInfo, AnimationInfo>();
        for (auto entity : players)
        {
            float throwingForce = 0.0f;
            MakeShotIfPossible(entity, throwingForce);
        }
    }
}

void PlayerControlSystem::HandlePlayerBuildingAction(const InputEventManager::EventInfo& eventInfo)
{
    auto event = eventInfo.originalEvent;

    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT)
    {
        auto physicsWorld = gameState.physicsWorld;

        auto windowPos = glm::vec2(event.button.x, event.button.y);
        auto worldPos = coordinatesTransformer.ScreenToWorld(windowPos);

        auto entity = registryWrapper.Create("buildingBlock");
        glm::vec2 sdlSize(10.0f, 10.0f);
        auto physicsBody = box2dBodyCreator.CreatePhysicsBody(entity, worldPos, sdlSize);
        registry.emplace<RenderingInfo>(entity, sdlSize, nullptr, SDL_Rect{}, ColorName::Green);
        registry.emplace<PhysicsInfo>(entity, physicsBody);
    }
};

void PlayerControlSystem::HandlePlayerWeaponDirection(const InputEventManager::EventInfo& eventInfo)
{
    auto event = eventInfo.originalEvent;

    if (event.type == SDL_MOUSEMOTION)
    {
        const auto& players = registry.view<PlayerInfo, PhysicsInfo>();
        for (auto entity : players)
        {
            const auto& [playerInfo, physicalBody] = players.get<PlayerInfo, PhysicsInfo>(entity);
            auto playerBody = physicalBody.bodyRAII->GetBody();

            glm::vec2 mouseWindowPos{event.motion.x, event.motion.y};
            glm::vec2 playerWindowPos = coordinatesTransformer.PhysicsToScreen(playerBody->GetPosition());
            glm::vec2 directionVec = mouseWindowPos - playerWindowPos;
            playerInfo.weaponDirection = glm::normalize(directionVec);
        }
    }
};

void PlayerControlSystem::HandlePlayerChangeWeapon(const InputEventManager::EventInfo& eventInfo)
{
    auto event = eventInfo.originalEvent;

    if (event.type != SDL_KEYDOWN)
        return;

    auto weaponIndex = event.key.keysym.sym - SDLK_1; // Get zero-based index of the weapon.
    auto weaponEnumRange = magic_enum::enum_values<WeaponType>();

    if (weaponIndex < 0 || weaponIndex >= static_cast<int>(weaponEnumRange.size()))
        return;

    const auto& players = registry.view<PlayerInfo>();
    for (auto entity : players)
    {
        auto& playerInfo = players.get<PlayerInfo>(entity);
        auto newWeapon = weaponEnumRange[weaponIndex];

        if (newWeapon == playerInfo.currentWeapon)
            continue;

        if (!playerInfo.weapons.contains(newWeapon))
        {
            MY_LOG_FMT(warn, "Player {} does not have {} weapon", playerInfo.number, newWeapon);
            continue;
        }

        playerInfo.currentWeapon = weaponEnumRange[weaponIndex];
        MY_LOG_FMT(trace, "Player {} changed weapon to {}", playerInfo.number, playerInfo.currentWeapon);
    }
};

void PlayerControlSystem::HandlePlayerEndPlayerContact(entt::entity entityA, entt::entity entityB)
{
    SetGroundContactFlagIfEntityIsPlayer(entityA, false);
    SetGroundContactFlagIfEntityIsPlayer(entityB, false);
};

void PlayerControlSystem::HandlePlayerBeginPlayerContact(entt::entity entityA, entt::entity entityB)
{
    SetGroundContactFlagIfEntityIsPlayer(entityA, true);
    SetGroundContactFlagIfEntityIsPlayer(entityB, true);
};

void PlayerControlSystem::SetGroundContactFlagIfEntityIsPlayer(entt::entity entity, bool value)
{
    auto playerInfo = registry.try_get<PlayerInfo>(entity);
    if (playerInfo)
    {
        playerInfo->countOfGroundContacts += value ? 1 : -1;
        MY_LOG_FMT(debug, "Player {} countOfGroundContacts: {}", playerInfo->number, playerInfo->countOfGroundContacts);
    }
};

entt::entity PlayerControlSystem::MakeShotIfPossible(entt::entity playerEntity, float throwingForce)
{
    if (!registry.all_of<PlayerInfo>(playerEntity))
    {
        MY_LOG_FMT(
            trace, "[MakeShotIfPossible] entity does not have all of the required components. Entity: {}",
            static_cast<int>(playerEntity));
        return entt::null;
    }

    auto& playerInfo = registry.get<PlayerInfo>(playerEntity);

    // Check if the throwing force is zero for the grenade.
    if (throwingForce <= 0 && playerInfo.currentWeapon == WeaponType::Grenade)
    {
        MY_LOG_FMT(
            trace, "[MakeShotIfPossible] Throwing force shouldn't be zero for weapon {}. Entity: {}, force: {}",
            playerInfo.currentWeapon, static_cast<int>(playerEntity), throwingForce);
        return entt::null;
    }

    // Check if player has weapon set as current.
    if (!playerInfo.weapons.contains(playerInfo.currentWeapon))
    {
        MY_LOG_FMT(
            trace, "[MakeShotIfPossible] Player does not have {} weapon set as current. Entity: {}",
            playerInfo.currentWeapon, static_cast<int>(playerEntity));
        return entt::null;
    }
    WeaponProps& currentWeaponProps = playerInfo.weapons.at(playerInfo.currentWeapon);

    // Check if player has ammo for the weapon.
    if (currentWeaponProps.ammoInClip == 0)
    {
        MY_LOG_FMT(
            trace, "[MakeShotIfPossible] Player does not have ammo in clip for the {} weapon. Entity: {}",
            playerInfo.currentWeapon, static_cast<int>(playerEntity));
        return entt::null;
    }

    // Check if player is in the reload process.
    if (currentWeaponProps.remainingReloadTime > 0)
    {
        MY_LOG_FMT(
            warn, "[MakeShotIfPossible] Player is in the reload process. Entity: {}", static_cast<int>(playerEntity));
        return entt::null;
    }

    // Check if player in the fire rate cooldown.
    if (currentWeaponProps.remainingFireRate > 0)
    {
        MY_LOG_FMT(
            trace, "[MakeShotIfPossible] Player is in the fire rate cooldown. Entity: {}",
            static_cast<int>(playerEntity));
        return entt::null;
    }

    // Update player ammo.
    currentWeaponProps.ammoInClip -= 1;
    currentWeaponProps.remainingFireRate = currentWeaponProps.fireRate;
    if (currentWeaponProps.ammoInClip == 0)
    {
        // TODO2: Start reloading sound and animation.
        currentWeaponProps.remainingReloadTime = currentWeaponProps.reloadTime;
    }

    // Calculate initial bullet speed.
    float initialBulletSpeed = currentWeaponProps.bulletEjectionForce / currentWeaponProps.bulletMass;
    initialBulletSpeed += throwingForce; // Add throwing force for the grenade with zero initial speed.
    initialBulletSpeed *= 40; // TODO2: Remove this magic number.

    // Create a bullet.
    auto bulletEntity = objectsFactory.CreateBullet(playerEntity, initialBulletSpeed);
    return bulletEntity;
};

void PlayerControlSystem::UpdateFireRateAndReloadTime(entt::entity playerEntity, float deltaTime)
{
    auto& playerInfo = registry.get<PlayerInfo>(playerEntity);
    for (auto& [weaponType, weaponProps] : playerInfo.weapons)
    {
        if (weaponProps.remainingFireRate > 0)
        {
            weaponProps.remainingFireRate -= deltaTime;
        }

        if (weaponProps.remainingReloadTime > 0)
        {
            weaponProps.remainingReloadTime -= deltaTime;

            if (weaponProps.remainingReloadTime <= 0)
            {
                // TODO2: Stop reloading sound and animation.
                weaponProps.ammoInClip = std::min(weaponProps.clipSize, weaponProps.ammoInStorage);
                weaponProps.ammoInStorage -= weaponProps.ammoInClip;
                MY_LOG_FMT(
                    trace, "Player {} reloaded weapon {}. Ammo in clip: {}, ammo in storage: {}", playerInfo.number,
                    weaponType, weaponProps.ammoInClip, weaponProps.ammoInStorage);
            }
        }
    }
};
