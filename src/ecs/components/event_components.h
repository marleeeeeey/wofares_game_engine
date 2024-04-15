#pragma once
#include <entt/entt.hpp>
#include <functional>

struct EventComponent
{
    enum class Opt
    {
        None = 0,
        CheckOnceAndRemove = 1 << 0,
        _entt_enum_as_bitmask
    };

    std::function<bool(void)> predicate;
    std::function<void(void)> onEvent;
    Opt options = Opt::None;
};
