#include "timers_control_system.h"
#include <ecs/components/event_components.h>

TimersControlSystem::TimersControlSystem(entt::registry& registry) : registry(registry)
{}

void TimersControlSystem::Update(float deltaTime)
{
    auto entityWithTimers = registry.view<TimeEventComponent>();
    for (auto& timerEntity : entityWithTimers)
    {
        auto& timerComponent = entityWithTimers.get<TimeEventComponent>(timerEntity);
        timerComponent.timeToActivation -= deltaTime;

        if (timerComponent.timeToActivation <= 0.0f && !timerComponent.isActivated)
        {
            timerComponent.isActivated = true;
            if (timerComponent.onTimerEnd)
                timerComponent.onTimerEnd(timerEntity);
        }
    }
}
