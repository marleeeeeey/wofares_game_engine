#include "audio_system.h"
#include <SDL_mixer.h>

AudioSystem::AudioSystem(ResourceManager& resourceManager) : resourceManager(resourceManager)
{}

void AudioSystem::PlayMusic(const std::string& musicName)
{
    auto musicRAII = resourceManager.GetMusic(musicName);
    Mix_PlayMusic(musicRAII->get(), -1);
};