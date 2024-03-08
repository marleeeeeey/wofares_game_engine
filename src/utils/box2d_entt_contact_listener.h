#pragma once
#include "utils/entt_registry_wrapper.h"
#include <box2d/box2d.h>
#include <entt/entt.hpp>
#include <optional>
#include <vector>

class Box2dEnttContactListener : public b2ContactListener
{
public:
    /**
     * IMPORTANT: ContactListener is called by Box2D during the simulation. It is not safe to modify the type of a Box2D
     * body. Attempting to change the type of a b2Body (e.g., from dynamic to static or vice versa) while the Box2D
     * world is locked (b2World::IsLocked() == true) is prohibited. The Box2D world is considered locked during the
     * processing of physical simulations, such as collision detection and contact resolution. Changing a body's type
     * under these conditions can lead to unpredictable behavior. Therefore, Box2D disallows type modifications to
     * bodies while the world is in this locked state.
     *
     * To safely change a body's type in reaction to collisions, consider deferring the change until after the
     * physics simulation step has completed.
     */
    using ContactListener = std::function<void(entt::entity, entt::entity)>;

    enum class ContactType
    {
        Begin,
        End,
        BeginSensor,
        EndSensor,
    };
private:
    struct EntityWithProperties
    {
        // Sensors in my game is thin rects below the player. Uses to determine if the entity on the floor.
        bool isSensor;
        entt::entity entity;
    };
public:
    Box2dEnttContactListener(EnttRegistryWrapper& registryWrapper);
    void SubscribeContact(ContactType contactType, ContactListener listener);
private:
    Box2dEnttContactListener(const Box2dEnttContactListener&) = delete;
    Box2dEnttContactListener& operator=(const Box2dEnttContactListener&) = delete;
    Box2dEnttContactListener(Box2dEnttContactListener&&) = delete;
    Box2dEnttContactListener& operator=(Box2dEnttContactListener&&) = delete;
private: // Interacting with Box2D. These methods are called by Box2D during the simulation.
    void BeginContact(b2Contact* contact) override;
    void EndContact(b2Contact* contact) override;
    std::optional<std::pair<EntityWithProperties, EntityWithProperties>> GetValidEntities(b2Contact* contact);
private:
    EnttRegistryWrapper& registryWrapper;
    entt::registry& registry;
    std::unordered_map<ContactType, std::vector<ContactListener>> contactListenersByType;
};
