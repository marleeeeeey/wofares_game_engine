#include "event_queue_system.h"
#include <SDL.h>
#include <imgui_impl_sdl2.h>

EventQueueSystem::EventQueueSystem(InputEventManager& inputEventManager) : inputEventManager(inputEventManager)
{}

void EventQueueSystem::Update(float deltaTime)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);

        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse || io.WantCaptureKeyboard)
        {
            // The mouse or keyboard event was processed by ImGui, do not process in the application.
            continue;
        }

        inputEventManager.UpdateRawEvent(event);
    }
    inputEventManager.UpdateСontinuousEvents(deltaTime);
}