#include "entt_registry_wrapper.h"
#include <entt/entt.hpp>
#include <my_cpp_utils/logger.h>

EnttRegistryWrapper::EnttRegistryWrapper(entt::registry& registry) : registry(registry)
{}

entt::entity EnttRegistryWrapper::Create([[maybe_unused]] const std::string& name)
{
    auto entity = registry.create();
#ifdef MY_DEBUG
    entityNamesById[entity] = name;
    MY_LOG(debug, "Creating entity id: {:>6} with name: {}", static_cast<uint32_t>(entity), name);
#endif // MY_DEBUG
    return entity;
}

void EnttRegistryWrapper::Destroy(entt::entity entity)
{
#ifdef MY_DEBUG
    auto& name = entityNamesById[entity];
    MY_LOG(debug, "Destroying entity id: {:>6} with name: {}", static_cast<uint32_t>(entity), name);
    removedEntityNamesById[entity] = name;
    entityNamesById.erase(entity);
#endif // MY_DEBUG
    registry.destroy(entity);
};

entt::registry& EnttRegistryWrapper::GetRegistry()
{
    return registry;
}

void EnttRegistryWrapper::LogAllEntitiesByTheirNames()
{
#ifdef MY_DEBUG
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
        MY_LOG(debug, "Entities with name: {} (count={}) have ids: {}", name, ids.size(), ids);
    }
#endif // MY_DEBUG
};

std::string EnttRegistryWrapper::TryGetName([[maybe_unused]] entt::entity entity)
{
#ifdef MY_DEBUG
    if (auto it = entityNamesById.find(entity); it != entityNamesById.end())
        return it->second;

    return MY_FMT(
        "Entity id: {:>6} REMOVED FROM REGISTRY. Last name was: {}.", static_cast<uint32_t>(entity),
        removedEntityNamesById[entity]);
#else
    return "";
#endif // MY_DEBUG
};