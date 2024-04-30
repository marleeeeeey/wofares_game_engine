#include "events_control_system.h"
#include <ecs/components/event_components.h>

EventsControlSystem::EventsControlSystem(entt::registry& registry) : registry(registry)
{}

void EventsControlSystem::Update()
{
    auto eventEntities = registry.view<EventComponent>();

    for (auto entity : eventEntities)
    {
        auto& eventComponent = eventEntities.get<EventComponent>(entity);
        if (eventComponent.predicate())
        {
            eventComponent.onEvent();

            if (!!(eventComponent.options & EventComponent::Opt::OneTimeOnPredicate))
            {
                registry.remove<EventComponent>(entity);
            }
        }
    }
}