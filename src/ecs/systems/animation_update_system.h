#pragma once
#include <entt/entt.hpp>
#include <utils/game_options.h>
#include <utils/resource_manager.h>

class AnimationUpdateSystem
{
    entt::registry& registry;
    GameOptions& gameState;
    ResourceManager& resourceManager;
public:
    AnimationUpdateSystem(entt::registry& registry, ResourceManager& resourceManager);
    void UpdateAnimationProgressForAllEntities(float deltaTime);
    void UpdatePlayerAnimationDirectionAndSpeed();
    void Update(float deltaTime);
};