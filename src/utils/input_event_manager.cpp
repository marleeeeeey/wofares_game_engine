#include "input_event_manager.h"
#include "SDL_stdinc.h"
#include "my_common_cpp_utils/Logger.h"

void InputEventManager::updateRawEvent(const SDL_Event& event)
{
    // Update the hold duration for keyboard buttons.
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
    {
        auto scancode = static_cast<SDL_Scancode>(event.key.keysym.scancode);
        if (event.type == SDL_KEYDOWN && !event.key.repeat)
        {
            keyboardButtonHoldInfo[scancode].isPressed = true;
            keyboardButtonHoldInfo[scancode].holdDuration = 0.0f;
            keyboardButtonHoldInfo[scancode].originalEvent = event;
        }
        else if (event.type == SDL_KEYUP)
        {
            keyboardButtonHoldInfo[scancode].isPressed = false;
        }
    }

    // Update the hold duration for mouse buttons.
    if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
    {
        Uint8 button = event.button.button;
        if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            mouseButtonHoldInfo[button].isPressed = true;
            mouseButtonHoldInfo[button].holdDuration = 0.0f;
            mouseButtonHoldInfo[button].originalEvent = event;
        }
        else if (event.type == SDL_MOUSEBUTTONUP)
        {
            mouseButtonHoldInfo[button].isPressed = false;
        }
    }

    // Notify usual rawListeners. They are not interested in hold duration.
    for (auto& listener : rawListeners)
    {
        listener(event);
    }
}

void InputEventManager::updateСontinuousEvents(float deltaTime)
{
    updateHoldDurations(deltaTime);

    // Notify rawListeners that are interested in hold duration.
    for (auto& [eventType, listeners] : continuousListeners)
    {
        for (auto& listener : listeners)
        {
            // TODO: Think how to remove duplication.
            for (auto& [key, eventInfo] : keyboardButtonHoldInfo)
            {
                if (eventInfo.isPressed && eventType == EventType::Down)
                {
                    listener(eventInfo);
                }
                else if (!eventInfo.isPressed && eventType == EventType::Up)
                {
                    listener(eventInfo);
                }
            }

            for (auto& [key, eventInfo] : mouseButtonHoldInfo)
            {
                if (eventInfo.isPressed && eventType == EventType::Down)
                {
                    listener(eventInfo);
                }
                else if (!eventInfo.isPressed && eventType == EventType::Up)
                {
                    listener(eventInfo);
                }
            }
        }
    }
}

void InputEventManager::subscribeRawListener(RawListener listener)
{
    rawListeners.push_back(listener);
    MY_LOG_FMT(info, "InputEventManager: RawListener added. Count of RawListeners: {}", rawListeners.size());
}

void InputEventManager::subscribeСontinuousListener(EventType eventType, СontinuousListener listener)
{
    continuousListeners[eventType].push_back(listener);
    MY_LOG_FMT(
        info, "InputEventManager: СontinuousListener added. Count of СontinuousListeners for {}: {}", eventType,
        continuousListeners[eventType].size());
}

void InputEventManager::updateHoldDurations(float deltaTime)
{
    // Update the hold duration for keyboard buttons.
    for (auto& [key, eventInfo] : keyboardButtonHoldInfo)
    {
        if (eventInfo.isPressed)
        {
            eventInfo.holdDuration += deltaTime;
        }
    }

    // Update the hold duration for mouse buttons.
    for (auto& [key, eventInfo] : mouseButtonHoldInfo)
    {
        if (eventInfo.isPressed)
        {
            eventInfo.holdDuration += deltaTime;
        }
    }
}
