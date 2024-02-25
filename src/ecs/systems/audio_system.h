#pragma once

#include "utils/resource_manager.h"
class AudioSystem
{
    ResourceManager& resourceManager;
public:
    AudioSystem(ResourceManager& resourceManager);
    void PlayMusic(const std::string& musicName);
};