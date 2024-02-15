#include "player_control_systems.h"
#include "ecs/systems/details/physics_body_creator.h"
#include "utils/input_event_manager.h"
#include <SDL.h>
#include <cassert>
#include <ecs/components/all_components.h>
#include <imgui_impl_sdl2.h>
#include <my_common_cpp_utils/Logger.h>
#include <utils/globals.h>

void SubscribePlayerControlSystem(entt::registry& registry, InputEventManager& inputEventManager)
{
    inputEventManager.subscribeСontinuousListener(
        InputEventManager::EventType::Down,
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

                if (originalEvent.key.keysym.scancode == SDL_SCANCODE_UP)
                {
                    body->ApplyForceToCenter(b2Vec2(0, -jumpForce), true);
                }

                if (originalEvent.key.keysym.scancode == SDL_SCANCODE_LEFT)
                {
                    body->ApplyForceToCenter(b2Vec2(-movingForce, 0), true);
                }

                if (originalEvent.key.keysym.scancode == SDL_SCANCODE_RIGHT)
                {
                    body->ApplyForceToCenter(b2Vec2(movingForce, 0), true);
                }
            }
        });

    // Subscribe when user up mouse left button after holding to spawn the granade and set the direction and force
    inputEventManager.subscribeСontinuousListener(
        InputEventManager::EventType::Up,
        [&registry](const InputEventManager::EventInfo& eventInfo)
        {
            if (eventInfo.originalEvent.key.keysym.scancode == SDL_SCANCODE_SPACE)
            {
                auto& gameState = registry.get<GameState>(registry.view<GameState>().front());
                auto physicsWorld = gameState.physicsWorld;
                const auto& players = registry.view<PlayerNumber, PhysicalBody>();

                for (auto entity : players)
                {
                    const auto& player = players.get<PhysicalBody>(entity).value;
                    auto playerBody = player->GetBody();
                    float force = eventInfo.holdDuration * 10.0f; // Force amplification coefficient.

                    b2Vec2 forceVec = b2Vec2(force, 0); // Force vector.
                    forceVec = b2Mul(playerBody->GetTransform().q, forceVec);

                    // Create a granade entity.
                    auto granadeEntity = registry.create();
                    glm::vec2 granadeSize(5.0f, 5.0f);
                    glm::vec2 playerSdlPosition(
                        playerBody->GetPosition().x * box2DtoSDL, playerBody->GetPosition().y * box2DtoSDL);

                    auto granadePhysicsBody = CreateDynamicPhysicsBody(physicsWorld, playerSdlPosition, granadeSize);
                    registry.emplace<SizeComponent>(granadeEntity, granadeSize);
                    registry.emplace<PhysicalBody>(granadeEntity, granadePhysicsBody);
                    registry.emplace<Granade>(granadeEntity);

                    // Apply the force to the granade.
                    granadePhysicsBody->GetBody()->ApplyLinearImpulseToCenter(forceVec, true);
                }
            }
        });
}
