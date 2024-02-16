#include "coordinates_transformer.h"

CoordinatesTransformer::CoordinatesTransformer(entt::registry& registry)
  : gameState(registry.get<GameState>(registry.view<GameState>().front())),
    box2DtoSDL(gameState.windowOptions.box2DtoSDL), sdlToBox2D(1.0f / box2DtoSDL)
{}

glm::vec2 CoordinatesTransformer::WorldToCamera(const glm::vec2& worldPos) const
{
    auto& rOpt = gameState.windowOptions;
    glm::vec2 transformedPosition = (worldPos - rOpt.cameraCenter) * rOpt.cameraScale + rOpt.windowSize / 2.0f;
    return transformedPosition;
}

glm::vec2 CoordinatesTransformer::CameraToWorld(const glm::vec2& cameraPos) const
{
    auto& rOpt = gameState.windowOptions;
    glm::vec2 worldPosition = ((cameraPos - rOpt.windowSize / 2.0f) / rOpt.cameraScale) + rOpt.cameraCenter;
    return worldPosition;
}

b2Vec2 CoordinatesTransformer::WorldToPhysics(const glm::vec2& worldPos) const
{
    return b2Vec2(worldPos.x * sdlToBox2D, worldPos.y * sdlToBox2D);
}

glm::vec2 CoordinatesTransformer::PhysicsToWorld(const b2Vec2& physicsPos) const
{
    return glm::vec2(physicsPos.x * box2DtoSDL, physicsPos.y * box2DtoSDL);
}

b2Vec2 CoordinatesTransformer::CameraToPhysics(const glm::vec2& cameraPos) const
{
    return WorldToPhysics(CameraToWorld(cameraPos));
}

glm::vec2 CoordinatesTransformer::PhysicsToCamera(const b2Vec2& physicsPos) const
{
    return WorldToCamera(PhysicsToWorld(physicsPos));
}
