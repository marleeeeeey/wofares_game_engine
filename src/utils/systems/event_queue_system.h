#pragma once
#include <utils/systems/input_event_manager.h>

class EventQueueSystem
{
    InputEventManager& inputEventManager;
public:
    EventQueueSystem(InputEventManager& inputEventManager);
    void Update(float deltaTime);
};