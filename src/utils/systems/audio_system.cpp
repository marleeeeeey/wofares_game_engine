#include "audio_system.h"
#include <SDL_mixer.h>

AudioSystem::AudioSystem(ResourceManager& resourceManager)
  : resourceManager(resourceManager), masterVolume(utils::GetConfig<float, "AudioSystem.masterVolume">())
{}

void AudioSystem::PlayMusic(const std::string& musicName)
{
    if (masterVolume == 0.0f)
        return;

    auto musicRAII = resourceManager.GetMusic(musicName);
    Mix_PlayMusic(musicRAII->get(), -1);
};

void AudioSystem::PlaySoundEffect(const std::string& soundEffectName)
{
    if (masterVolume == 0.0f)
        return;

    auto soundEffectRAII = resourceManager.GetSoundEffect(soundEffectName);
    Mix_PlayChannel(-1, soundEffectRAII->get(), 0);
};