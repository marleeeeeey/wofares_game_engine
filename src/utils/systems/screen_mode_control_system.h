#pragma once
#include <utils/RAII/sdl_RAII.h>
#include <utils/systems/input_event_manager.h>

class ScreenModeControlSystem
{
    InputEventManager& inputEventManager;
    SDLWindowRAII& window;
public:
    ScreenModeControlSystem(InputEventManager& inputEventManager, SDLWindowRAII& window);
private:
    void SubscribeToInputEvents();
};