#include "input_systems.h"
#include <SDL.h>
#include <ecs/components/all_components.h>
#include <imgui_impl_sdl2.h>
#include <my_common_cpp_utils/Logger.h>

void InputSystem(entt::registry& registry)
{
    const float desiredVelocity = 5000.0f;

    const Uint8* currentKeyStates = SDL_GetKeyboardState(nullptr);

    const auto& players = registry.view<PlayerNumber, PhysicalBody>();
    for (auto entity : players)
    {
        const auto& [playerNumber, physicalBody] = players.get<PlayerNumber, PhysicalBody>(entity);
        auto body = physicalBody.value->GetBody();
        auto vel = body->GetLinearVelocity();

        if (currentKeyStates[SDL_SCANCODE_UP])
        {
            b2Vec2 impulse(0.0f, -body->GetMass() * desiredVelocity);
            body->ApplyLinearImpulseToCenter(impulse, true);
        }

        if (currentKeyStates[SDL_SCANCODE_LEFT])
        {
            body->SetLinearVelocity(b2Vec2(-desiredVelocity, vel.y));
            // body->ApplyForceToCenter(b2Vec2(-desiredVelocity, 0), true);
        }

        if (currentKeyStates[SDL_SCANCODE_RIGHT])
        {
            body->SetLinearVelocity(b2Vec2(desiredVelocity, vel.y));
            // body->ApplyForceToCenter(b2Vec2(desiredVelocity, 0), true);
        }
    }
}
