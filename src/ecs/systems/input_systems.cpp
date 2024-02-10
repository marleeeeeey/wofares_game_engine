#include "input_systems.h"
#include <SDL.h>
#include <ecs/components/all_components.h>
#include <imgui_impl_sdl2.h>
#include <my_common_cpp_utils/Logger.h>

void InputSystem(entt::registry& registry)
{
    const float jumpVelocity = -600.0f;
    const float moveSpeed = 300.0f;

    const Uint8* currentKeyStates = SDL_GetKeyboardState(nullptr);

    auto playerWithVelocity = registry.view<PlayerNumber>();
    for (auto entity : playerWithVelocity)
    {
        if (currentKeyStates[SDL_SCANCODE_UP])
        {
            // vel.value.y = jumpVelocity;
        }
        if (currentKeyStates[SDL_SCANCODE_LEFT])
        {
            // vel.value.x = -moveSpeed;
        }
        else if (currentKeyStates[SDL_SCANCODE_RIGHT])
        {
            // vel.value.x = moveSpeed;
        }
        else
        {
            // vel.value.x = 0;
        }
    }
}
