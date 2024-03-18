#pragma once
#include <SDL.h>
#include <utils/animation.h>

struct AnimationComponent
{
    Animation animation; // Frames of the animation.
    float currentFrameTime = 0; // Time in seconds from the start of the current frame.
    size_t currentFrameIndex = 0; // Index of the current frame.
    bool isPlaying = false; // Is the animation playing.
    bool loop = true; // Should the animation loop.
    SDL_RendererFlip flip = SDL_FLIP_NONE; // Flip of the animation.
    float speedFactor = 1.0f;
public: // Helpers.
    inline glm::vec2 GetHitboxSize() const { return glm::vec2(animation.hitboxRect->w, animation.hitboxRect->h); }
};