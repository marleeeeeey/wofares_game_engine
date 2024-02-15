#include "objects_factory.h"
#include "ecs/systems/details/coordinates_transformer.h"
#include <ecs/components/all_components.h>
#include <ecs/systems/details/physics_body_creator.h>

ObjectsFactory::ObjectsFactory(entt::registry& registry)
  : registry(registry), gameState(registry.get<GameState>(registry.view<GameState>().front())),
    physicsWorld(gameState.physicsWorld)
{}

entt::entity ObjectsFactory::createPlayer(const glm::vec2& playerSdlWorldPos)
{
    CoordinatesTransformer coordinatesTransformer(registry);
    auto gap = gameState.physicsOptions.gapBetweenPhysicalAndVisual;

    glm::vec2 playerSdlWorldSize{10, 10};
    glm::vec2 playerSdlBBox = playerSdlWorldSize - glm::vec2{gap, gap};

    auto entity = registry.create();
    registry.emplace<SdlSizeComponent>(entity, playerSdlWorldSize);
    registry.emplace<PlayerNumber>(entity);
    registry.emplace<PlayerDirection>(entity);
    auto playerPhysicsBody =
        CreateDynamicPhysicsBody(coordinatesTransformer, physicsWorld, playerSdlWorldPos, playerSdlBBox);
    registry.emplace<PhysicalBody>(entity, playerPhysicsBody);
    return entity;
}
