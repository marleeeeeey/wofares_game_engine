#pragma once
#include "utils/box2d/box2d_body_tuner.h"
#include <entt/entt.hpp>
#include <utils/game_options.h>
#include <utils/resources/resource_manager.h>

class AnimationUpdateSystem
{
    entt::registry& registry;
    GameOptions& gameState;
    ResourceManager& resourceManager;
    Box2dBodyTuner box2dBodyTuner;
public:
    AnimationUpdateSystem(entt::registry& registry, ResourceManager& resourceManager);
    void UpdateAnimationProgressForAllEntities(float deltaTime);
    void UpdatePlayerAnimationDirectionAndSpeed();
    void Update(float deltaTime);
};