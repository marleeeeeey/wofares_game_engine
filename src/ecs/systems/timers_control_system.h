#pragma once
#include <entt/entt.hpp>

class TimersControlSystem
{
    entt::registry& registry;
public:
    TimersControlSystem(entt::registry& registry);
    void Update(float deltaTime);
};