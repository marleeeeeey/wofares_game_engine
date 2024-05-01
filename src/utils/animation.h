#pragma once
#include <ecs/components/rendering_components.h>
#include <optional>
#include <vector>

struct AnimationFrame
{
    TileComponent tileComponent; // Rendering information for the frame.
    float duration; // Duration of the frame in seconds.
};

struct Animation
{
    std::vector<AnimationFrame> frames; // Frames of the animation.
    std::optional<SDL_Rect> hitboxRect; // Hitbox of the animation.
};
