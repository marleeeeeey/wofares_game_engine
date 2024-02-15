#include "coordinates_transformer.h"

CoordinatesTransformer::CoordinatesTransformer(entt::registry& registry)
  : gameState(registry.get<GameState>(registry.view<GameState>().front()))
{}

glm::vec2 CoordinatesTransformer::WorldToCamera(const glm::vec2& worldPos)
{
    auto& rOpt = gameState.renderingOptions;
    glm::vec2 transformedPosition = (worldPos - rOpt.cameraCenter) * rOpt.cameraScale + rOpt.windowSize / 2.0f;
    return transformedPosition;
}

glm::vec2 CoordinatesTransformer::CameraToWorld(const glm::vec2& cameraPos)
{
    auto& rOpt = gameState.renderingOptions;
    glm::vec2 worldPosition = ((cameraPos - rOpt.windowSize / 2.0f) / rOpt.cameraScale) + rOpt.cameraCenter;
    return worldPosition;
}
