#pragma once
#include <box2d/box2d.h>
#include <entt/entt.hpp>
#include <optional>
#include <vector>

class Box2dEnttContactListener : public b2ContactListener
{
public:
    using ContactListener = std::function<void(entt::entity, entt::entity)>;

    enum class ContactType
    {
        Begin,
        End
    };
public:
    Box2dEnttContactListener(entt::registry& registry);
    void SubscribeContact(ContactType contactType, ContactListener listener);
private:
    Box2dEnttContactListener(const Box2dEnttContactListener&) = delete;
    Box2dEnttContactListener& operator=(const Box2dEnttContactListener&) = delete;
    Box2dEnttContactListener(Box2dEnttContactListener&&) = delete;
    Box2dEnttContactListener& operator=(Box2dEnttContactListener&&) = delete;
private: // Interacting with Box2D.
    void BeginContact(b2Contact* contact) override;
    void EndContact(b2Contact* contact) override;
    std::optional<std::pair<entt::entity, entt::entity>> GetValidEntities(b2Contact* contact);
private:
    entt::registry& registry;
    std::vector<ContactListener> beginContactListeners;
    std::vector<ContactListener> endContactListeners;
};
