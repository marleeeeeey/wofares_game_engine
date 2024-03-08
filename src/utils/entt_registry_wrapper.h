#pragma once
#include <entt/entt.hpp>

class EnttRegistryWrapper
{
    entt::registry& registry;
    std::map<entt::entity, std::string> entityNamesById;
public:
    explicit EnttRegistryWrapper(entt::registry& registry);
    EnttRegistryWrapper(const EnttRegistryWrapper&) = delete;
    EnttRegistryWrapper& operator=(const EnttRegistryWrapper&) = delete;
public: // Methods for debug - use in client code.
    entt::entity Create(const std::string& name);
    void Destroy(entt::entity entity);
    void LogAllEntitiesByTheirNames();
    // Get original registry.
    entt::registry& GetRegistry();
};