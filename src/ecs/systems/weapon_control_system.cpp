#include "weapon_control_system.h"
#include <ecs/components/game_components.h>
#include <utils/glm_box2d_conversions.h>

WeaponControlSystem::WeaponControlSystem(entt::registry& registry, float deltaTime)
  : registry(registry), gameState(registry.get<GameState>(registry.view<GameState>().front())), deltaTime(deltaTime)
{
    GrenadeExpiryControl();
}

void WeaponControlSystem::GrenadeExpiryControl()
{
    auto viewGrenedes = registry.view<Grenade, PhysicalBody>();

    std::vector<entt::entity> entitiesToDestroy;

    for (auto& grenadeEntity : viewGrenedes)
    {
        auto& grenade = viewGrenedes.get<Grenade>(grenadeEntity);
        auto& grenadeBody = viewGrenedes.get<PhysicalBody>(grenadeEntity);
        grenade.timeToExplode -= deltaTime;

        if (grenade.timeToExplode <= 0.0f)
        {
            auto viewTargets = registry.view<PhysicalBody>();
            for (auto& targetEntity : viewTargets)
            {
                if (targetEntity == grenadeEntity)
                    continue;

                auto& targetBody = viewTargets.get<PhysicalBody>(targetEntity);

                // Calculate distance between grenade and target.
                const auto& grenedePhysicsPos = grenadeBody.value->GetBody()->GetPosition();
                const auto& targetPhysicsPos = targetBody.value->GetBody()->GetPosition();
                float distance = utils::distance(grenedePhysicsPos, targetPhysicsPos);

                if (distance <= grenade.explosionRadius)
                {
                    entitiesToDestroy.push_back(targetEntity);
                }
            }

            entitiesToDestroy.push_back(grenadeEntity);
        }
    }

    for (auto& entity : entitiesToDestroy)
    {
        registry.destroy(entity);
    }
}
