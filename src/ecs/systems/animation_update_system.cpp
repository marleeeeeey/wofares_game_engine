#include "animation_update_system.h"

AnimationUpdateSystem::AnimationUpdateSystem(entt::registry& registry, ResourceManager& resourceManager)
  : registry(registry), resourceManager(resourceManager),
    gameState(registry.get<GameState>(registry.view<GameState>().front()))
{}

void AnimationUpdateSystem::Update(float deltaTime)
{
    auto view = registry.view<AnimationInfo>();

    for (auto entity : view)
    {
        auto& animation = view.get<AnimationInfo>(entity);

        if (animation.isPlaying)
        {
            animation.currentFrameTime += deltaTime;

            if (animation.currentFrameTime >= animation.frames[animation.currentFrameIndex].duration)
            {
                animation.currentFrameTime -= animation.frames[animation.currentFrameIndex].duration;
                animation.currentFrameIndex = (animation.currentFrameIndex + 1) % animation.frames.size();

                // Stop the animation if it's not looped and the last frame is reached.
                if (!animation.loop && animation.currentFrameIndex == 0)
                {
                    animation.isPlaying = false;
                }
            }
        }
    }
}
