#pragma once
#include <entt/entt.hpp>
#include <utils/game_options.h>
#include <utils/resource_manager.h>

class AnimationUpdateSystem
{
    entt::registry& registry;
    GameOptions& gameState;
public:
    AnimationUpdateSystem(entt::registry& registry);
    void UpdateAnimationProgressForAllEntities(float deltaTime);
    void UpdatePlayerAnimationDirectionAndSpeed();
    void Update(float deltaTime);
};