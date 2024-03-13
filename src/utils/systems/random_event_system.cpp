#include "random_event_system.h"

RandomEventSystem::RandomEventSystem(entt::registry& registry, AudioSystem& audioSystem)
  : registry(registry), gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())),
    audioSystem(audioSystem)
{
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

void RandomEventSystem::Update(float deltaTime)
{
    elapsedTime += deltaTime;
    auto& soundOptions = gameState.soundOptions;

    if (elapsedTime >= soundOptions.nextSoundEventTime)
    {
        ProduceEvilLaugh();
        soundOptions.nextSoundEventTime = elapsedTime + GenerateNextEventTime(soundOptions.randomSoundEventInterval);
    }
};

void RandomEventSystem::ProduceEvilLaugh()
{
    audioSystem.PlaySoundEffect("evil_laugh");
};

float RandomEventSystem::GenerateNextEventTime(float averageInterval)
{
    std::uniform_real_distribution<float> distribution(averageInterval * 0.5, averageInterval * 1.5);
    return distribution(rng);
}
