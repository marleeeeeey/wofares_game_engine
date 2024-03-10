#include "animation_update_system.h"

AnimationUpdateSystem::AnimationUpdateSystem(entt::registry& registry, ResourceManager& resourceManager)
  : registry(registry), gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())),
    resourceManager(resourceManager)
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
        auto& animationInfo = view.get<AnimationInfo>(entity);

        if (animationInfo.isPlaying)
        {
            animationInfo.currentFrameTime += deltaTime * animationInfo.speedFactor * 0.5;

            // TODO1: Unify using the modulo operator in interface.
            auto safeIndex = animationInfo.currentFrameIndex % animationInfo.animation.frames.size();

            if (animationInfo.currentFrameTime >= animationInfo.animation.frames[safeIndex].duration)
            {
                animationInfo.currentFrameTime -= animationInfo.animation.frames[safeIndex].duration;
                animationInfo.currentFrameIndex =
                    (animationInfo.currentFrameIndex + 1) % animationInfo.animation.frames.size();

                // Stop the animation if it's not looped and the last frame is reached.
                if (!animationInfo.loop && animationInfo.currentFrameIndex == 0)
                {
                    animationInfo.isPlaying = false;
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

        if (speed > 0.1f)
        {
            animationInfo.animation = resourceManager.GetAnimation("player_walk", "Run");
            animationInfo.speedFactor = std::min(speed, 2.5f); // Limit max speed.
        }
        else
        {
            animationInfo.animation = resourceManager.GetAnimation("player_walk", "Idle");
            animationInfo.speedFactor = 1.0f;
        }

        // Change the animation direction based on the player's direction.
        if (playerInfo.weaponDirection.x < 0)
            animationInfo.flip = SDL_FLIP_HORIZONTAL;
        else if (playerInfo.weaponDirection.x > 0)
            animationInfo.flip = SDL_FLIP_NONE;
    }
};