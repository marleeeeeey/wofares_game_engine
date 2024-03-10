#include "coordinates_transformer.h"

CoordinatesTransformer::CoordinatesTransformer(entt::registry& registry)
  : gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())),
    box2DtoSDL(gameState.windowOptions.box2DtoSDL), sdlToBox2D(1.0f / box2DtoSDL)
{}

glm::vec2 CoordinatesTransformer::WorldToCamera(const glm::vec2& worldPos) const
{
    auto& rOpt = gameState.windowOptions;
    glm::vec2 transformedPosition = (worldPos - rOpt.cameraCenterSdl) * rOpt.cameraScale + rOpt.windowSize / 2.0f;
    return transformedPosition;
}

glm::vec2 CoordinatesTransformer::CameraToWorld(const glm::vec2& cameraPos) const
{
    auto& rOpt = gameState.windowOptions;
    glm::vec2 worldPosition = ((cameraPos - rOpt.windowSize / 2.0f) / rOpt.cameraScale) + rOpt.cameraCenterSdl;
    return worldPosition;
}

b2Vec2 CoordinatesTransformer::WorldToPhysics(const glm::vec2& worldPos) const
{
    return b2Vec2(worldPos.x * sdlToBox2D, worldPos.y * sdlToBox2D);
}

float CoordinatesTransformer::WorldToPhysics(float worldValue) const
{
    return worldValue * sdlToBox2D;
};

glm::vec2 CoordinatesTransformer::PhysicsToWorld(const b2Vec2& physicsPos) const
{
    return glm::vec2(physicsPos.x * box2DtoSDL, physicsPos.y * box2DtoSDL);
}

float CoordinatesTransformer::PhysicsToWorld(float physicsValue) const
{
    return physicsValue * box2DtoSDL;
};

b2Vec2 CoordinatesTransformer::CameraToPhysics(const glm::vec2& cameraPos) const
{
    return WorldToPhysics(CameraToWorld(cameraPos));
}

glm::vec2 CoordinatesTransformer::PhysicsToCamera(const b2Vec2& physicsPos) const
{
    return WorldToCamera(PhysicsToWorld(physicsPos));
}