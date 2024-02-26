#pragma once
#include "ecs/components/game_state_component.h"
#include <entt/entt.hpp>
#include <random>
#include <utils/audio_system.h>

class RandomEventSystem
{
    AudioSystem& audioSystem;
    entt::registry& registry;
    GameState& gameState;
    std::default_random_engine rng;
    float elapsedTime;
public:
    RandomEventSystem(entt::registry& registry, AudioSystem& audioSystem);
    void Update(float deltaTime);
private:
    void ProduceEvilLaugh();
    float GenerateNextEventTime(float averageInterval);
};