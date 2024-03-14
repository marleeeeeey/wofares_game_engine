#pragma once
#include "utils/game_options.h"
#include <entt/entt.hpp>
#include <random>
#include <utils/systems/audio_system.h>

class RandomEventSystem
{
    AudioSystem& audioSystem;
    entt::registry& registry;
    GameOptions& gameState;
    std::default_random_engine rng;
    float elapsedTime{0.0f};
    float nextSoundEventTime{0.0f}; // Time of the next random sound event.
public:
    RandomEventSystem(entt::registry& registry, AudioSystem& audioSystem);
    void Update(float deltaTime);
private:
    void ProduceEvilLaugh();
    float GenerateNextEventTime(float averageInterval);
};