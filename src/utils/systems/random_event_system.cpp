#include "random_event_system.h"
#include <my_cpp_utils/config.h>

RandomEventSystem::RandomEventSystem(entt::registry& registry, AudioSystem& audioSystem)
  : registry(registry), gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())),
    audioSystem(audioSystem)
{
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    nextSoundEventTime = utils::GetConfig<float, "RandomEventSystem.randomSoundEventInterval_sec">();
}

void RandomEventSystem::Update(float deltaTime)
{
    elapsedTime += deltaTime;

    if (elapsedTime >= nextSoundEventTime)
    {
        ProduceEvilLaugh();
        auto& randomSoundEventInterval = utils::GetConfig<float, "RandomEventSystem.randomSoundEventInterval_sec">();
        nextSoundEventTime = elapsedTime + GenerateNextEventTime(randomSoundEventInterval);
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
