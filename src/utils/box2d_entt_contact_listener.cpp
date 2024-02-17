#include "box2d_entt_contact_listener.h"
#include "my_common_cpp_utils/Logger.h"

Box2dEnttContactListener::Box2dEnttContactListener(entt::registry& registry) : registry(registry)
{}

void Box2dEnttContactListener::BeginContact(b2Contact* contact)
{
    if (auto validEntities = GetValidEntities(contact))
    {
        auto [entityA, entityB] = *validEntities;
        for (auto& listener : beginContactListeners)
        {
            listener(entityA, entityB);
        }
    }
}

void Box2dEnttContactListener::EndContact(b2Contact* contact)
{
    if (auto validEntities = GetValidEntities(contact))
    {
        auto [entityA, entityB] = *validEntities;
        for (auto& listener : endContactListeners)
        {
            listener(entityA, entityB);
        }
    }
}

std::optional<std::pair<entt::entity, entt::entity>> Box2dEnttContactListener::GetValidEntities(b2Contact* contact)
{
    auto* bodyA = contact->GetFixtureA()->GetBody();
    auto* bodyB = contact->GetFixtureB()->GetBody();

    auto pointerA = bodyA->GetUserData().pointer;
    auto pointerB = bodyB->GetUserData().pointer;
    if (pointerA == 0 || pointerB == 0)
    {
        MY_LOG_FMT(warn, "One of the bodies has no user data. pointerA: {}, pointerB: {}", pointerA, pointerB);
        return std::nullopt;
    }

    auto entityA = static_cast<entt::entity>(pointerA);
    auto entityB = static_cast<entt::entity>(pointerB);

    if (registry.valid(entityA) && registry.valid(entityB))
    {
        return std::make_pair(entityA, entityB);
    }

    MY_LOG_FMT(warn, "One of the entities is not valid. entityA: {}, entityB: {}", entityA, entityB);
    return std::nullopt;
}

void Box2dEnttContactListener::SubscribeContact(ContactType contactType, ContactListener listener)
{
    if (contactType == ContactType::Begin)
    {
        beginContactListeners.push_back(listener);
        MY_LOG_FMT(info, "Subscribed to begin contact listener. Count of listeners: {}", beginContactListeners.size());
    }
    else
    {
        endContactListeners.push_back(listener);
        MY_LOG_FMT(info, "Subscribed to end contact listener. Count of listeners: {}", endContactListeners.size());
    }
}
