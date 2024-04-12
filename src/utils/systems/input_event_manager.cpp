#include "input_event_manager.h"
#include <my_cpp_utils/logger.h>

void InputEventManager::UpdateRawEvent(const SDL_Event& event)
{
    UpdateButtonHoldStaticInfo(event);

    EventInfo eventInfo;
    eventInfo.isPressed = event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONDOWN;
    eventInfo.originalEvent = event;

    // Notify usual rawListeners. They are not interested in hold duration.
    auto& rawListeners = continuousListeners[EventType::RawSdlEvent];
    for (auto& listener : rawListeners)
    {
        listener(eventInfo);
    }

    // Notify buttonPressListeners.
    if (event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONDOWN)
    {
        for (auto& listener : continuousListeners[EventType::ButtonPress])
        {
            listener(eventInfo);
        }
    }

    // Notify buttonReleaseListeners.
    if (event.type == SDL_KEYUP || event.type == SDL_MOUSEBUTTONUP)
    {
        for (auto& listener : continuousListeners[EventType::ButtonRelease])
        {
            listener(eventInfo);
        }
    }
}

void InputEventManager::UpdateСontinuousEvents(float deltaTime)
{
    UpdateHoldDurations(deltaTime);

    for (auto& [scanCode, eventInfo] : keyboardButtonHoldInfo)
    {
        for (auto& listener : continuousListeners[EventType::ButtonHold])
        {
            if (eventInfo.info.isPressed)
            {
                listener(eventInfo.info);
            }
        }
    }

    for (auto& [button, eventInfo] : mouseButtonHoldInfo)
    {
        for (auto& listener : continuousListeners[EventType::ButtonHold])
        {
            if (eventInfo.info.isPressed)
            {
                listener(eventInfo.info);
            }
        }
    }

    for (auto& [scanCode, eventInfo] : keyboardButtonHoldInfo)
    {
        for (auto& listener : continuousListeners[EventType::ButtonReleaseAfterHold])
        {
            if (!eventInfo.firedToAll && !eventInfo.info.isPressed)
            {
                listener(eventInfo.info);
                MY_LOG(debug, "Keyboard button {} is released. Event sent to listener.", scanCode);
            }
        }
        eventInfo.firedToAll = true; // This flag should be set to true after all listeners are notified.
    }

    for (auto& [button, eventInfo] : mouseButtonHoldInfo)
    {
        for (auto& listener : continuousListeners[EventType::ButtonReleaseAfterHold])
        {
            if (!eventInfo.firedToAll && !eventInfo.info.isPressed)
            {
                listener(eventInfo.info);
                MY_LOG(debug, "Mouse button {} is released. Event sent to listener.", button);
            }
        }
        eventInfo.firedToAll = true; // This flag should be set to true after all listeners are notified.
    }
}

void InputEventManager::Subscribe(EventListener listener)
{
    Subscribe(EventType::RawSdlEvent, listener);
}

void InputEventManager::Subscribe(EventType eventType, EventListener listener)
{
    auto& listeners = continuousListeners[eventType];
    listeners.push_back(listener);
    MY_LOG(debug, "InputEventManager: {} listener added. Count={}.", eventType, listeners.size());
}

void InputEventManager::UpdateHoldDurations(float deltaTime)
{
    // Update the hold duration for keyboard buttons.
    for (auto& [key, eventInfo] : keyboardButtonHoldInfo)
    {
        if (eventInfo.info.isPressed)
        {
            eventInfo.info.holdDuration += deltaTime;
        }
    }

    // Update the hold duration for mouse buttons.
    for (auto& [key, eventInfo] : mouseButtonHoldInfo)
    {
        if (eventInfo.info.isPressed)
        {
            eventInfo.info.holdDuration += deltaTime;
        }
    }
}

void InputEventManager::UpdateButtonHoldStaticInfo(const SDL_Event& event)
{
    // Update the hold duration for keyboard buttons.
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
    {
        auto scancode = static_cast<SDL_Scancode>(event.key.keysym.scancode);
        keyboardButtonHoldInfo[scancode].firedToAll = false;
        if (event.type == SDL_KEYDOWN && !event.key.repeat)
        {
            keyboardButtonHoldInfo[scancode].info.isPressed = true;
            keyboardButtonHoldInfo[scancode].info.holdDuration = 0.0f;
            keyboardButtonHoldInfo[scancode].info.originalEvent = event;
        }
        else if (event.type == SDL_KEYUP)
        {
            keyboardButtonHoldInfo[scancode].info.isPressed = false;
        }
    }

    // Update the hold duration for mouse buttons.
    if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
    {
        Uint8 button = event.button.button;
        mouseButtonHoldInfo[button].firedToAll = false;
        if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            mouseButtonHoldInfo[button].info.isPressed = true;
            mouseButtonHoldInfo[button].info.holdDuration = 0.0f;
            mouseButtonHoldInfo[button].info.originalEvent = event;
            MY_LOG(debug, "Mouse button {} is pressed. Size of cashe {}", button, mouseButtonHoldInfo.size());
        }
        else if (event.type == SDL_MOUSEBUTTONUP)
        {
            mouseButtonHoldInfo[button].info.isPressed = false;
            MY_LOG(debug, "Mouse button {} is released. Size of cashe {}", button, mouseButtonHoldInfo.size());
        }
    }
}