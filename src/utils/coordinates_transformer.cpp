#include "coordinates_transformer.h"

CoordinatesTransformer::CoordinatesTransformer(entt::registry& registry)
  : gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())),
    box2DtoSDL(gameState.windowOptions.box2DtoSDL), sdlToBox2D(1.0f / box2DtoSDL)
{}

glm::vec2 CoordinatesTransformer::WorldToScreen(const glm::vec2& worldPos, Type type) const
{
    auto& windowOpt = gameState.windowOptions;

    if (type == Type::Length)
        return worldPos * windowOpt.cameraScale;

    glm::vec2 transformedPosition =
        (worldPos - windowOpt.cameraCenterSdl) * windowOpt.cameraScale + windowOpt.windowSize / 2.0f;
    return transformedPosition;
}

float CoordinatesTransformer::WorldToScreen(float worldValue) const
{
    return worldValue * gameState.windowOptions.cameraScale;
};

glm::vec2 CoordinatesTransformer::ScreenToWorld(const glm::vec2& screenPos, Type type) const
{
    auto& windowOpt = gameState.windowOptions;

    if (type == Type::Length)
        return screenPos / windowOpt.cameraScale;

    glm::vec2 worldPosition =
        ((screenPos - windowOpt.windowSize / 2.0f) / windowOpt.cameraScale) + windowOpt.cameraCenterSdl;
    return worldPosition;
}

float CoordinatesTransformer::ScreenToWorld(float screenValue) const
{
    return screenValue / gameState.windowOptions.cameraScale;
};

b2Vec2 CoordinatesTransformer::WorldToPhysics(const glm::vec2& worldPos, [[maybe_unused]] Type type) const
{
    return b2Vec2(worldPos.x * sdlToBox2D, worldPos.y * sdlToBox2D);
}

float CoordinatesTransformer::WorldToPhysics(float worldValue) const
{
    return worldValue * sdlToBox2D;
};

glm::vec2 CoordinatesTransformer::PhysicsToWorld(const b2Vec2& physicsPos, [[maybe_unused]] Type type) const
{
    return glm::vec2(physicsPos.x * box2DtoSDL, physicsPos.y * box2DtoSDL);
}

float CoordinatesTransformer::PhysicsToWorld(float physicsValue) const
{
    return physicsValue * box2DtoSDL;
};

b2Vec2 CoordinatesTransformer::ScreenToPhysics(const glm::vec2& screenPos, Type type) const
{
    return WorldToPhysics(ScreenToWorld(screenPos, type), type);
}

float CoordinatesTransformer::ScreenToPhysics(float screenValue) const
{
    return WorldToPhysics(ScreenToWorld(screenValue));
};

glm::vec2 CoordinatesTransformer::PhysicsToScreen(const b2Vec2& physicsPos, Type type) const
{
    return WorldToScreen(PhysicsToWorld(physicsPos, type), type);
}

float CoordinatesTransformer::PhysicsToScreen(float physicsValue) const
{
    return WorldToScreen(PhysicsToWorld(physicsValue));
};