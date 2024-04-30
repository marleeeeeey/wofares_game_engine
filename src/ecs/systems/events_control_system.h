#pragma once
#include <entt/entt.hpp>

class EventsControlSystem
{
    entt::registry& registry;
public:
    EventsControlSystem(entt::registry& registry);
    void Update();
};