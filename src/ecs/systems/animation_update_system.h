#pragma once
#include <entt/entt.hpp>
#include <utils/game_options.h>
#include <utils/resource_manager.h>


class AnimationUpdateSystem
{
    entt::registry& registry;
    ResourceManager& resourceManager; // TODO: unused. Remove later if not needed.
    GameOptions& gameState;
public:
    AnimationUpdateSystem(entt::registry& registry, ResourceManager& resourceManager);
    void UpdateAnimationProgressForAllEntities(float deltaTime);
    void UpdatePlayerAnimationDirectionAndSpeed();
    void Update(float deltaTime);
};