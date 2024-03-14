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
    gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())), transformer(registry),
    box2dBodyCreator(registry), contactListener(contactListener), objectsFactory(objectsFactory)
{
    SubscribeToInputEvents();
    SubscribeToContactListener();
}

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
            const auto& playerInfo = registry.get<PlayerInfo>(entity);

            // Map current weapon to specific force.
            std::unordered_map<PlayerInfo::Weapon, float> weaponToForce = {
                {PlayerInfo::Weapon::Bazooka, std::min(eventInfo.holdDuration * 10.0f, 3.0f)},
                {PlayerInfo::Weapon::Grenade, std::min(eventInfo.holdDuration * 10.0f, 3.0f)}};

            if (weaponToForce.find(playerInfo.currentWeapon) == weaponToForce.end())
                continue;

            auto force = weaponToForce[playerInfo.currentWeapon];
            objectsFactory.CreateBullet(entity, force);
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
            const auto& playerInfo = registry.get<PlayerInfo>(entity);

            // Map current weapon to specific force.
            std::unordered_map<PlayerInfo::Weapon, float> weaponToForce = {
                {PlayerInfo::Weapon::Uzi, 1.0}, {PlayerInfo::Weapon::Pistol, 0.5}};

            if (weaponToForce.find(playerInfo.currentWeapon) == weaponToForce.end())
                continue;

            auto force = weaponToForce[playerInfo.currentWeapon];
            objectsFactory.CreateBullet(entity, force);
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
        auto worldPos = transformer.ScreenToWorld(windowPos);

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
            glm::vec2 playerWindowPos = transformer.PhysicsToScreen(playerBody->GetPosition());
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
    auto weaponEnumRange = magic_enum::enum_values<PlayerInfo::Weapon>();

    if (weaponIndex < 0 || weaponIndex >= static_cast<int>(weaponEnumRange.size()))
        return;

    const auto& players = registry.view<PlayerInfo>();
    for (auto entity : players)
    {
        auto& playerInfo = players.get<PlayerInfo>(entity);
        auto newWeapon = weaponEnumRange[weaponIndex];

        if (newWeapon == playerInfo.currentWeapon)
            continue;

        playerInfo.currentWeapon = weaponEnumRange[weaponIndex];
        MY_LOG_FMT(info, "Player {} changed weapon to {}", playerInfo.number, playerInfo.currentWeapon);
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