#pragma once
#include <utils/input_event_manager.h>
#include <utils/sdl_RAII.h>

class ScreenModeControlSystem
{
    InputEventManager& inputEventManager;
    SDLWindowRAII& window;
public:
    ScreenModeControlSystem(InputEventManager& inputEventManager, SDLWindowRAII& window);
private:
    void SubscribeToInputEvents();
};