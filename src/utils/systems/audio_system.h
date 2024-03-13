#pragma once
#include <my_common_cpp_utils/config.h>
#include <utils/resources/resource_manager.h>

class AudioSystem
{
    ResourceManager& resourceManager;
    const float& masterVolume;
public:
    AudioSystem(ResourceManager& resourceManager);
    void PlayMusic(const std::string& musicName);
    void PlaySoundEffect(const std::string& soundEffectName);
};