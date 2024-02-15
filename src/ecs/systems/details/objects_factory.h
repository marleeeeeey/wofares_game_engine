#pragma once
#include <entt/entt.hpp>

class ObjectsFactory
{
    entt::registry& registry;
public:
    ObjectsFactory(entt::registry& registry) : registry(registry) {}
};