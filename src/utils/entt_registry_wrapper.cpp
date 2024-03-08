#include "entt_registry_wrapper.h"
#include "entt/entity/fwd.hpp"
#include <my_common_cpp_utils/logger.h>
#include <vector>

EnttRegistryWrapper::EnttRegistryWrapper(entt::registry& registry) : registry(registry)
{}

entt::entity EnttRegistryWrapper::Create(const std::string& name)
{
    auto entity = registry.create();
    entityNamesById[entity] = name;
    MY_LOG_FMT(debug, "Creating entity id: {:>6} with name: {}", static_cast<uint32_t>(entity), name);
    return entity;
}

void EnttRegistryWrapper::Destroy(entt::entity entity)
{
    MY_LOG_FMT(
        debug, "Destroying entity id: {:>6} with name: {}", static_cast<uint32_t>(entity), entityNamesById[entity]);
    entityNamesById.erase(entity);
    registry.destroy(entity);
};

entt::registry& EnttRegistryWrapper::GetRegistry()
{
    return registry;
}

void EnttRegistryWrapper::LogAllEntitiesByTheirNames()
{
    std::map<std::string, std::vector<entt::entity>> entitiesByName;

    for (const auto& [entity, name] : entityNamesById)
        entitiesByName[name].push_back(entity);

    for (const auto& [name, entities] : entitiesByName)
    {
        std::string ids;
        for (const auto& entity : entities)
            ids += std::to_string(static_cast<uint32_t>(entity)) + ", ";
        ids.pop_back();
        ids.pop_back();
        MY_LOG_FMT(debug, "Entities with name: {} (count={}) have ids: {}", name, ids.size(), ids);
    }
};

std::string EnttRegistryWrapper::TryGetName(entt::entity entity)
{
    if (auto it = entityNamesById.find(entity); it != entityNamesById.end())
        return it->second;
    return "OBJECT REMOVED FROM REGISTRY";
};