#pragma once
#include <SDL_mixer.h>
#include <string>

class SDLAudioInitializerRAII
{
public:
    SDLAudioInitializerRAII();
    ~SDLAudioInitializerRAII();
    SDLAudioInitializerRAII(const SDLAudioInitializerRAII&) = delete;
    SDLAudioInitializerRAII& operator=(const SDLAudioInitializerRAII&) = delete;
};

class MusicRAII
{
public:
    MusicRAII(const std::string& filename);
    ~MusicRAII();
    MusicRAII(const MusicRAII&) = delete;
    MusicRAII& operator=(const MusicRAII&) = delete;

    Mix_Music* get() const { return music; }
private:
    Mix_Music* music = nullptr;
};

class SoundEffectRAII
{
public:
    SoundEffectRAII(const std::string& filename);
    ~SoundEffectRAII();
    SoundEffectRAII(const SoundEffectRAII&) = delete;
    SoundEffectRAII& operator=(const SoundEffectRAII&) = delete;

    Mix_Chunk* get() const { return chunk; }
private:
    Mix_Chunk* chunk = nullptr;
};