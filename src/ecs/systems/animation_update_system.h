#pragma once
#include <ecs/components/game_state_component.h>
#include <entt/entt.hpp>
#include <utils/resource_manager.h>

class AnimationUpdateSystem
{
    entt::registry& registry;
    ResourceManager& resourceManager; // TODO: unused. Remove later if not needed.
    GameState& gameState;
public:
    AnimationUpdateSystem(entt::registry& registry, ResourceManager& resourceManager);
    void UpdateAnimationProgressForAllEntities(float deltaTime);
    void UpdatePlayerAnimationDirection();
    void Update(float deltaTime);
};