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
        RawSdlEvent,
        ButtonPress,
        ButtonRelease,
        ButtonHold,
        ButtonReleaseAfterHold
    };
public: // Methods to subscribe to the events.
    using EventListener = std::function<void(const EventInfo&)>;
    // Subscribe to raw SDL events.
    void Subscribe(EventListener listener);
    // Subscribe to specific event type.
    void Subscribe(EventType eventType, EventListener listener);
public: // Methods to translate SDL events to the EventInfo and notify listeners.
    // This method should be called on every SDL_Event in the event queue.
    void UpdateRawEvent(const SDL_Event& event);
    // This method should be called every frame after updateRawEvent.
    // It updates hold durations and notifies listeners with hold durations.
    void UpdateСontinuousEvents(float deltaTime);
public: // Hacks.
    // Reset all the hold durations and flags. Should be called on level restart.
    // TODO1: This looks like a bad design. The main reason is not clear.
    void Reset();
private:
    struct PrivateEventInfo
    {
        // Used for Release events. This is because of assimetry of press and release events.
        // Hold time is not needed for release events.
        bool firedToAll = false;
        EventInfo info;
    };

    void UpdateHoldDurations(float deltaTime);
    void UpdateButtonHoldStaticInfo(const SDL_Event& event);

    // Listeners for the events.
    std::unordered_map<EventType, std::vector<EventListener>> continuousListeners;
    // Status of the buttons.
    std::unordered_map<SDL_Scancode, PrivateEventInfo> keyboardButtonHoldInfo;
    std::unordered_map<Uint8, PrivateEventInfo> mouseButtonHoldInfo;
};
