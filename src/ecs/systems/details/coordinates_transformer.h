#pragma once
#include <ecs/components/all_components.h>
#include <entt/entt.hpp>

class CoordinatesTransformer
{
    GameState& gameState;
public:
    CoordinatesTransformer(entt::registry& registry);
    glm::vec2 WorldToCamera(const glm::vec2& worldPos);
    glm::vec2 CameraToWorld(const glm::vec2& cameraPos);
};