#pragma once
#include <entt/entt.hpp>
#include <functional>

struct EventComponent
{
    enum class Opt
    {
        AnyTimeOnPredicate = 0,
        OneTimeOnPredicate = 1 << 0,
        _entt_enum_as_bitmask
    };

    // TODO2: Support multiple independent events.
    std::function<bool(void)> predicate;
    std::function<void(void)> onEvent;
    Opt options = Opt::AnyTimeOnPredicate;
};

struct TimeEventComponent
{
    // TODO2: Support multiple independent timers.
    float timeToActivation = 3.0f; // Time in seconds to call the callback.
    std::function<void(entt::entity)> onTimerEnd; // Callback on timer end.
    bool isActivated = false; // Is the timer activated.
};
