#include "player_control_systems.h"
#include <SDL.h>
#include <ecs/components/game_components.h>
#include <ecs/systems/details/coordinates_transformer.h>
#include <ecs/systems/details/physics_body_creator.h>
#include <imgui_impl_sdl2.h>
#include <my_common_cpp_utils/Logger.h>
#include <utils/input_event_manager.h>


void SubscribePlayerControlSystem(entt::registry& registry, InputEventManager& inputEventManager)
{
    inputEventManager.SubscribeСontinuousListener(
        InputEventManager::EventType::ButtonHold,
        [&registry](const InputEventManager::EventInfo& eventInfo)
        {
            const float movingForce = 10.0f;
            const float jumpForce = 30.0f;

            auto& originalEvent = eventInfo.originalEvent;

            const auto& players = registry.view<PlayerNumber, PhysicalBody>();
            for (auto entity : players)
            {
                const auto& [playerNumber, physicalBody] = players.get<PlayerNumber, PhysicalBody>(entity);
                auto body = physicalBody.value->GetBody();
                auto vel = body->GetLinearVelocity();

                if (originalEvent.key.keysym.scancode == SDL_SCANCODE_W)
                {
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
        });

    // Subscribe when user up mouse left button after holding to spawn the granade and set the direction and force
    inputEventManager.SubscribeСontinuousListener(
        InputEventManager::EventType::ButtonReleaseAfterHold,
        [&registry](const InputEventManager::EventInfo& eventInfo)
        {
            if (eventInfo.originalEvent.button.button == SDL_BUTTON_LEFT)
            {
                auto& gameState = registry.get<GameState>(registry.view<GameState>().front());
                auto physicsWorld = gameState.physicsWorld;
                const auto& players =
                    registry.view<PlayerNumber, PhysicalBody, PlayersWeaponDirection, SdlSizeComponent>();
                CoordinatesTransformer transformer(registry);

                for (auto entity : players)
                {
                    const auto& player = players.get<PhysicalBody>(entity).value;
                    const auto& weaponDirection = players.get<PlayersWeaponDirection>(entity).value;
                    const auto& playerSize = players.get<SdlSizeComponent>(entity).value;
                    auto playerBody = player->GetBody();

                    // Clamp the force to the maximum value.
                    float force = std::min(eventInfo.holdDuration * 20.0f, 6.0f);

                    // Rotate the force vector to the direction of the weapon.
                    b2Vec2 forceVec = b2Vec2(force, 0);
                    forceVec = b2Mul(b2Rot(atan2(weaponDirection.y, weaponDirection.x)), forceVec);

                    // Calculate the position of the granade slightly in front of the player.
                    glm::vec2 granadeWorldSize(5.0f, 5.0f);
                    glm::vec2 playerWorldPos = transformer.PhysicsToWorld(playerBody->GetPosition());
                    glm::vec2 granadeWorldPos = playerWorldPos + weaponDirection * playerSize.x / 1.5f;

                    // Create the granade entity and set the physics body.
                    auto granadeEntity = registry.create();
                    auto granadePhysicsBody =
                        CreateDynamicPhysicsBody(transformer, physicsWorld, granadeWorldPos, granadeWorldSize);
                    registry.emplace<SdlSizeComponent>(granadeEntity, granadeWorldSize);
                    registry.emplace<PhysicalBody>(granadeEntity, granadePhysicsBody);
                    registry.emplace<Granade>(granadeEntity);

                    // Apply the force to the granade.
                    granadePhysicsBody->GetBody()->ApplyLinearImpulseToCenter(forceVec, true);
                }
            }
        });

    // Subscribe when user press mouse right button to create a static body at the mouse position
    inputEventManager.SubscribeRawListener(
        [&registry](const SDL_Event& event)
        {
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT)
            {
                auto& gameState = registry.get<GameState>(registry.view<GameState>().front());
                auto physicsWorld = gameState.physicsWorld;
                CoordinatesTransformer transformer(registry);

                auto windowPos = glm::vec2(event.button.x, event.button.y);

                auto worldPos = transformer.CameraToWorld(windowPos);

                auto entity = registry.create();
                glm::vec2 size(10.0f, 10.0f);
                auto physicsBody = CreateStaticPhysicsBody(transformer, physicsWorld, worldPos, size);
                registry.emplace<SdlSizeComponent>(entity, size);
                registry.emplace<PhysicalBody>(entity, physicsBody);
                registry.emplace<Bridge>(entity);
            }
        });

    // subscribe player on mouse movement to set the direction of the weapon
    inputEventManager.SubscribeRawListener(
        [&registry](const SDL_Event& event)
        {
            if (event.type == SDL_MOUSEMOTION)
            {
                auto& gameState = registry.get<GameState>(registry.view<GameState>().front());
                const auto& players = registry.view<PlayerNumber, PlayersWeaponDirection, PhysicalBody>();
                CoordinatesTransformer transformer(registry);

                for (auto entity : players)
                {
                    auto [playerNumber, direction, physicalBody] =
                        players.get<PlayerNumber, PlayersWeaponDirection, PhysicalBody>(entity);
                    auto playerBody = physicalBody.value->GetBody();

                    glm::vec2 mouseWindowPos{event.motion.x, event.motion.y};
                    glm::vec2 playerWindowPos = transformer.PhysicsToCamera(playerBody->GetPosition());
                    glm::vec2 directionVec = mouseWindowPos - playerWindowPos;
                    direction.value = glm::normalize(directionVec);
                }
            }
        });
}
