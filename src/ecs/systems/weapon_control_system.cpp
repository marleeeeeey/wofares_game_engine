#include "weapon_control_system.h"
#include <ecs/components/game_components.h>

WeaponControlSystem::WeaponControlSystem(entt::registry& registry, float deltaTime)
  : registry(registry), gameState(registry.get<GameState>(registry.view<GameState>().front())), deltaTime(deltaTime)
{
    GrenadeExpiryControl();
}

void WeaponControlSystem::GrenadeExpiryControl()
{
    auto view = registry.view<Grenade>();

    for (auto entity : view)
    {
        auto& grenade = view.get<Grenade>(entity);
        grenade.timeToExplode -= deltaTime;
        if (grenade.timeToExplode <= 0.0f)
        {
            registry.destroy(entity);
        }
    }
}
