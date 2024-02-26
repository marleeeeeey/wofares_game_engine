#include "animation_update_system.h"

AnimationUpdateSystem::AnimationUpdateSystem(entt::registry& registry, ResourceManager& resourceManager)
  : registry(registry), resourceManager(resourceManager),
    gameState(registry.get<GameState>(registry.view<GameState>().front()))
{}

void AnimationUpdateSystem::Update(float deltaTime)
{
    UpdateAnimationProgressForAllEntities(deltaTime);
    UpdatePlayerAnimationDirectionAndSpeed();
}

void AnimationUpdateSystem::UpdateAnimationProgressForAllEntities(float deltaTime)
{
    auto view = registry.view<AnimationInfo>();

    for (auto entity : view)
    {
        auto& animation = view.get<AnimationInfo>(entity);

        if (animation.isPlaying)
        {
            animation.currentFrameTime += deltaTime * animation.speedFactor * 0.5;

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
};

void AnimationUpdateSystem::UpdatePlayerAnimationDirectionAndSpeed()
{
    auto view = registry.view<AnimationInfo, PlayerInfo, PhysicsInfo>();

    for (auto entity : view)
    {
        const auto& [animationInfo, playerInfo, physicsInfo] = view.get<AnimationInfo, PlayerInfo, PhysicsInfo>(entity);

        // Change the animation speed based on the player's speed.
        auto body = physicsInfo.bodyRAII->GetBody();
        auto vel = body->GetLinearVelocity();
        float speed = glm::length(glm::vec2(vel.x, vel.y));
        animationInfo.speedFactor = std::min(speed, 2.5f);

        // Change the animation direction based on the player's direction.
        if (playerInfo.weaponDirection.x < 0)
            animationInfo.flip = SDL_FLIP_HORIZONTAL;
        else if (playerInfo.weaponDirection.x > 0)
            animationInfo.flip = SDL_FLIP_NONE;
    }
};