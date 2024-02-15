#include "event_queue_system.h"
#include <SDL.h>
#include <imgui_impl_sdl2.h>

void EventQueueSystem(InputEventManager& inputEventManager, float deltaTime)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        inputEventManager.UpdateRawEvent(event);
    }
    inputEventManager.UpdateСontinuousEvents(deltaTime);
}