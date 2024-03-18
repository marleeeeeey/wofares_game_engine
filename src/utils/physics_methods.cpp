#include "physics_methods.h"
#include <ecs/components/physics_components.h>
#include <utils/glm_box2d_conversions.h>
#include <utils/math_utils.h>

PhysicsMethods::PhysicsMethods(entt::registry& registry)
  : registry(registry), gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())),
    box2dBodyCreator(registry), coordinatesTransformer(registry)
{}

void PhysicsMethods::ApplyForceToPhysicalBodies(
    std::vector<entt::entity> physicalEntities, const glm::vec2& forceCenterWorld, float force)
{
    auto forceCenterPhysics = coordinatesTransformer.WorldToPhysics(forceCenterWorld);

    for (auto& entity : physicalEntities)
    {
        auto originalObjPhysicsInfo = registry.get<PhysicsComponent>(entity).bodyRAII->GetBody();
        const b2Vec2& posPhysics = originalObjPhysicsInfo->GetPosition();

        // Make target body as dynamic.
        originalObjPhysicsInfo->SetType(b2_dynamicBody);

        // Apply force to the target.
        // Force direction is from grenade to target. Inside. This greate interesting effect.
        auto forceVec = -(posPhysics - forceCenterPhysics) * force;
        originalObjPhysicsInfo->ApplyForceToCenter(forceVec, true);
    }
}
