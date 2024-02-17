#pragma once
#include <utils/input_event_manager.h>

class EventQueueSystem
{
    InputEventManager& inputEventManager;
public:
    EventQueueSystem(InputEventManager& inputEventManager);
    void Update(float deltaTime);
};