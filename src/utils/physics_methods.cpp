#include "physics_methods.h"
#include <utils/box2d_helpers.h>
#include <utils/glm_box2d_conversions.h>

PhysicsMethods::PhysicsMethods(entt::registry& registry)
  : registry(registry), gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())),
    box2dBodyCreator(registry), coordinatesTransformer(registry)
{}

void PhysicsMethods::ApplyForceToPhysicalBodies(
    std::vector<entt::entity> physicalEntities, const b2Vec2& grenadePhysicsPos, float force)
{
    auto physicsWorld = gameState.physicsWorld;
    auto gap = gameState.physicsOptions.gapBetweenPhysicalAndVisual;

    for (auto& entity : physicalEntities)
    {
        auto originalObjPhysicsInfo = registry.get<PhysicsInfo>(entity).bodyRAII->GetBody();
        const b2Vec2& physicsPos = originalObjPhysicsInfo->GetPosition();

        // Make target body as dynamic.
        originalObjPhysicsInfo->SetType(b2_dynamicBody);

        // Calculate distance between grenade and target.
        float distance = utils::CaclDistance(grenadePhysicsPos, physicsPos);

        // Apply force to the target.
        // Force direction is from grenade to target. Inside. This greate interesting effect.
        auto forceVec = -(physicsPos - grenadePhysicsPos) * force;
        originalObjPhysicsInfo->ApplyForceToCenter(forceVec, true);
    }
}
