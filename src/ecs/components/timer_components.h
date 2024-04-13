#pragma once
#include <entt/entt.hpp>
#include <functional>

// Timer component. Used to setup user callbacks in time for any action.
struct TimerComponent
{
    float timeToActivation = 3.0f; // Time in seconds to call the callback.
    std::function<void(entt::entity)> onTimerEnd; // Callback on timer end.
    bool isActivated = false; // Is the timer activated.
};
