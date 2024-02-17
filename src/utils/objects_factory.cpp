#include "objects_factory.h"
#include <ecs/components/game_components.h>
#include <utils/box2d_body_creator.h>
#include <utils/coordinates_transformer.h>

ObjectsFactory::ObjectsFactory(entt::registry& registry)
  : registry(registry), gameState(registry.get<GameState>(registry.view<GameState>().front())),
    physicsWorld(gameState.physicsWorld)
{}

entt::entity ObjectsFactory::createPlayer(const glm::vec2& sdlPos)
{
    CoordinatesTransformer coordinatesTransformer(registry);
    auto gap = gameState.physicsOptions.gapBetweenPhysicalAndVisual;

    glm::vec2 playerSdlSize{10, 10};
    glm::vec2 playerSdlBBox = playerSdlSize - glm::vec2{gap, gap};

    auto entity = registry.create();
    registry.emplace<RenderingInfo>(entity, playerSdlSize);
    registry.emplace<PlayerInfo>(entity);
    auto playerPhysicsBody =
        CreateDynamicPhysicsBody(entity, coordinatesTransformer, physicsWorld, sdlPos, playerSdlBBox);
    registry.emplace<PhysicsInfo>(entity, playerPhysicsBody);
    return entity;
}
