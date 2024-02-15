#pragma once
#include <SDL_events.h>
#include <functional>

class InputEventManager
{
public:
    struct EventInfo
    {
        float holdDuration = 0.0f;
        bool isPressed = false; // Currently pressed or not.
        SDL_Event originalEvent{}; // The original event. Have to use to get the original event type and scancode.
    };

    enum EventType
    {
        ButtonHold,
        ButtonReleaseAfterHold
    };

    using RawListener = std::function<void(const SDL_Event&)>;
    using СontinuousListener = std::function<void(const EventInfo&)>;
    void SubscribeRawListener(RawListener listener);
    void SubscribeСontinuousListener(EventType eventType, СontinuousListener listener);
    // This method should be called on every SDL_Event in the event queue.
    void UpdateRawEvent(const SDL_Event& event);
    // This method should be called every frame after updateRawEvent.
    // It updates hold durations and notifies listeners with hold durations.
    void UpdateСontinuousEvents(float deltaTime);
private:
    struct PrivateEventInfo
    {
        bool firedToAll = false; // Used for UP events. This is because of assimetry of DOWN and UP events.
        EventInfo info;
    };

    void UpdateHoldDurations(float deltaTime);

    std::vector<RawListener> rawListeners;
    std::unordered_map<EventType, std::vector<СontinuousListener>> continuousListeners;
    std::unordered_map<SDL_Scancode, PrivateEventInfo> keyboardButtonHoldInfo; // TODO: array.
    std::unordered_map<Uint8, PrivateEventInfo> mouseButtonHoldInfo; // TODO: array
};
