#include "keyboard_state_systems.h"
#include <SDL.h>
#include <ecs/components/all_components.h>
#include <imgui_impl_sdl2.h>
#include <my_common_cpp_utils/Logger.h>

void KeyboardStateSystem(entt::registry& registry)
{
    const float movingForce = 10.0f;
    const float jumpForce = 30.0f;

    const Uint8* currentKeyStates = SDL_GetKeyboardState(nullptr);

    const auto& players = registry.view<PlayerNumber, PhysicalBody>();
    for (auto entity : players)
    {
        const auto& [playerNumber, physicalBody] = players.get<PlayerNumber, PhysicalBody>(entity);
        auto body = physicalBody.value->GetBody();
        auto vel = body->GetLinearVelocity();

        if (currentKeyStates[SDL_SCANCODE_UP])
        {
            body->ApplyForceToCenter(b2Vec2(0, -jumpForce), true);
        }

        if (currentKeyStates[SDL_SCANCODE_LEFT])
        {
            body->ApplyForceToCenter(b2Vec2(-movingForce, 0), true);
        }

        if (currentKeyStates[SDL_SCANCODE_RIGHT])
        {
            body->ApplyForceToCenter(b2Vec2(movingForce, 0), true);
        }
    }
}
