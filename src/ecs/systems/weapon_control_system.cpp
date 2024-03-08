#include "weapon_control_system.h"
#include "box2d/b2_body.h"
#include "entt/entity/fwd.hpp"
#include "utils/entt_registry_wrapper.h"
#include "utils/sdl_colors.h"
#include <SDL_rect.h>
#include <box2d/b2_math.h>
#include <ecs/components/game_components.h>
#include <my_common_cpp_utils/logger.h>
#include <my_common_cpp_utils/math_utils.h>
#include <utils/box2d_body_creator.h>
#include <utils/box2d_entt_contact_listener.h>
#include <utils/box2d_helpers.h>
#include <utils/glm_box2d_conversions.h>
#include <utils/texture_process.h>
#include <vector>

WeaponControlSystem::WeaponControlSystem(
    EnttRegistryWrapper& registryWrapper, Box2dEnttContactListener& contactListener, AudioSystem& audioSystem)
  : registryWrapper(registryWrapper), registry(registryWrapper.GetRegistry()),
    gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())), contactListener(contactListener),
    audioSystem(audioSystem)
{
    contactListener.SubscribeContact(
        Box2dEnttContactListener::ContactType::Begin,
        [this](entt::entity entityA, entt::entity entityB)
        {
            for (const auto& entity : {entityA, entityB})
            {
                // If the entity contains the ContactExplosionComponent.
                if (!registry.all_of<ContactExplosionComponent>(entity))
                    continue;

                auto& contactExplosion = registry.get<ContactExplosionComponent>(entity);
                if (contactExplosion.spawnSafeTime <= 0.0f)
                {
                    // Update Box2D object is not allowed in the contact listener. Because Box2D is in simulation
                    // step. So, we need to store entities in the queue and update them in the main loop.
                    explosionEntities.push(entity);
                }
            }
        });
}

void WeaponControlSystem::UpdateTimerExplosionComponents()
{
    auto timersView = registry.view<TimerExplosionComponent>();
    for (auto& timerEntity : timersView)
    {
        auto& timerExplosion = timersView.get<TimerExplosionComponent>(timerEntity);
        timerExplosion.timeToExplode -= deltaTime;

        if (timerExplosion.timeToExplode <= 0.0f)
        {
            TryToRunExplosionImpactComponent(timerEntity);
        }
    }
}

std::vector<entt::entity> WeaponControlSystem::GetPhysicalBodiesInRaduis(
    const b2Vec2& grenadePhysicsPos, float grenadeExplosionRadius, std::optional<b2BodyType> bodyType)
{
    auto viewTargets = registry.view<PhysicsInfo>();
    std::vector<entt::entity> targetsVector = {viewTargets.begin(), viewTargets.end()};
    return GetPhysicalBodiesInRaduis(targetsVector, grenadePhysicsPos, grenadeExplosionRadius, bodyType);
}

void WeaponControlSystem::ApplyForceToPhysicalBodies(
    std::vector<entt::entity> physicalEntities, const b2Vec2& grenadePhysicsPos, float force)
{
    auto physicsWorld = gameState.physicsWorld;
    auto gap = gameState.physicsOptions.gapBetweenPhysicalAndVisual;
    Box2dBodyCreator box2dBodyCreator(registry);
    CoordinatesTransformer coordinatesTransformer(registry);

    for (auto& entity : physicalEntities)
    {
        auto originalObjPhysicsInfo = registry.get<PhysicsInfo>(entity).bodyRAII->GetBody();
        auto& originalObjRenderingInfo = registry.get<RenderingInfo>(entity);
        const b2Vec2& physicsPos = originalObjPhysicsInfo->GetPosition();

        // TODO replace original texture in rederingInfo to the random explosion fragment. Prepare png's for it.

        // Make target body as dynamic.
        originalObjPhysicsInfo->SetType(b2_dynamicBody);

        // Calculate distance between grenade and target.
        float distance = utils::distance(grenadePhysicsPos, physicsPos);

        // Apply force to the target.
        // Force direction is from grenade to target. Inside. This greate interesting effect.
        auto forceVec = -(physicsPos - grenadePhysicsPos) * force;
        originalObjPhysicsInfo->ApplyForceToCenter(forceVec, true);
    }
}

void WeaponControlSystem::Update(float deltaTime)
{
    this->deltaTime = deltaTime;
    UpdateTimerExplosionComponents();
    UpdateContactExplosionComponentTimer();
    ProcessExplosionEntitiesQueue();
}

void WeaponControlSystem::OnBazookaContactWithTile(entt::entity bazookaEntity, entt::entity tileEntity)
{
    MY_LOG(info, "Bazooka contact with tile");
};

void WeaponControlSystem::TryToRunExplosionImpactComponent(entt::entity explosionEntity)
{
    auto explosionImpact = registry.try_get<ExplosionImpactComponent>(explosionEntity);
    auto physicsInfo = registry.try_get<PhysicsInfo>(explosionEntity);

    if (explosionImpact && physicsInfo)
    {
        const b2Vec2& grenadePhysicsPos = physicsInfo->bodyRAII->GetBody()->GetPosition();
        float radiusCoef = 1.2f; // TODO: hack. Need to calculate it based on the texture size. Because position is
                                 // calculated from the center of the texture.
        auto staticOriginalBodies =
            GetPhysicalBodiesInRaduis(grenadePhysicsPos, explosionImpact->radius * radiusCoef, b2_staticBody);

        // TODO: maybe better split entities to static and dynamic. And apply force only to dynamic.
        SDL_Point cellSize = {2, 2}; // TODO: move to the game options.
        auto splittedEntities = AddAndReturnSplittedPhysicalEntetiesToWorld(staticOriginalBodies, cellSize);

        // TODO: add to debug options.
        bool debugMiniDistructionEnabled = false; // true by default.
        bool debugForceEnabled = false; // true by default.
        bool debugDestroyOriginalObjectEmidiatly = true; // false by default.

        if (debugMiniDistructionEnabled)
        {
            auto staticMicroBodiesToDestroy =
                GetPhysicalBodiesInRaduis(splittedEntities, grenadePhysicsPos, explosionImpact->radius, b2_staticBody);
            for (auto& entity : staticMicroBodiesToDestroy)
                registryWrapper.Destroy(entity);
        }

        if (debugForceEnabled)
        {
            ApplyForceToPhysicalBodies(staticOriginalBodies, grenadePhysicsPos, explosionImpact->force);
            StartCollisionDisableTimer(staticOriginalBodies);
        }

        if (debugDestroyOriginalObjectEmidiatly)
        {
            for (auto& entity : staticOriginalBodies)
            {
                registryWrapper.Destroy(entity);
            }
        }

        registryWrapper.Destroy(explosionEntity);
        audioSystem.PlaySoundEffect("explosion");
    }
};
void WeaponControlSystem::ProcessExplosionEntitiesQueue()
{
    while (!explosionEntities.empty())
    {
        auto entity = explosionEntities.front();
        TryToRunExplosionImpactComponent(entity);
        explosionEntities.pop();
    }
};

void WeaponControlSystem::StartCollisionDisableTimer(const std::vector<entt::entity>& physicalEntities)
{
    for (auto& entity : physicalEntities)
    {
        // Exclude players from the list.
        if (registry.any_of<PlayerInfo>(entity))
            continue;

        if (utils::RandomTrue(gameState.levelOptions.colisionDisableProbability))
            registry.emplace_or_replace<CollisionDisableTimerComponent>(entity);
    }
};

void WeaponControlSystem::UpdateContactExplosionComponentTimer()
{
    auto contactExplosionsView = registry.view<ContactExplosionComponent>();
    for (auto& entity : contactExplosionsView)
    {
        auto& contactExplosion = contactExplosionsView.get<ContactExplosionComponent>(entity);
        contactExplosion.spawnSafeTime -= deltaTime;
    }
};

std::vector<entt::entity> WeaponControlSystem::AddAndReturnSplittedPhysicalEntetiesToWorld(
    const std::vector<entt::entity>& physicalEntities, SDL_Point cellSize)
{
    auto physicsWorld = gameState.physicsWorld;
    auto gap = gameState.physicsOptions.gapBetweenPhysicalAndVisual;
    Box2dBodyCreator box2dBodyCreator(registry);
    CoordinatesTransformer coordinatesTransformer(registry);
    glm::vec2 cellSizeGlm(cellSize.x, cellSize.y);

    std::vector<entt::entity> splittedEntities;

    for (auto& entity : physicalEntities)
    {
        auto originalObjPhysicsInfo = registry.get<PhysicsInfo>(entity).bodyRAII->GetBody();
        auto& originalObjRenderingInfo = registry.get<RenderingInfo>(entity);
        const b2Vec2& physicsPos = originalObjPhysicsInfo->GetPosition();
        const glm::vec2 originalObjWorldPos = coordinatesTransformer.PhysicsToWorld(physicsPos);

        // Check if the original object is big enough to be splitted.
        if (originalObjRenderingInfo.textureRect.w < cellSize.x || originalObjRenderingInfo.textureRect.h < cellSize.y)
            continue;

        auto originalRectPosInTexture =
            glm::vec2(originalObjRenderingInfo.textureRect.x, originalObjRenderingInfo.textureRect.y);

        auto textureRects = DivideRectByCellSize(originalObjRenderingInfo.textureRect, cellSize);
        for (auto& rect : textureRects)
        {
            // Fill rendering info for the pixel.
            RenderingInfo pixelRenderingInfo;

            // TODO: add to debug options.
            bool debugColoredPixelsRandomly = true; // true by default.

            if (debugColoredPixelsRandomly)
            {
                pixelRenderingInfo.colorName = GetRandomColorName();
            }
            else
            {
                pixelRenderingInfo.texturePtr = originalObjRenderingInfo.texturePtr;
                pixelRenderingInfo.textureRect = rect;
            }

            pixelRenderingInfo.sdlSize = cellSizeGlm;

            // Create physics body for the pixel.
            auto pixelRectPosInTexture = glm::vec2(rect.x, rect.y);
            glm::vec2 pixelWorldPosition = originalObjWorldPos + (pixelRectPosInTexture - originalRectPosInTexture) -
                cellSizeGlm * 1.5; // TODO: here is a hack with 1.5.
            glm::vec2 pixelTileSize(rect.w - gap, rect.h - gap);
            auto pixelPhysicsBody = box2dBodyCreator.CreatePhysicsBody(entity, pixelWorldPosition, pixelTileSize);

            // Create entity for the pixel with appropriate components.
            auto pixelEntity = registryWrapper.Create("pixelTile");
            registry.emplace<RenderingInfo>(pixelEntity, pixelRenderingInfo);
            registry.emplace<PhysicsInfo>(pixelEntity, pixelPhysicsBody);
            splittedEntities.push_back(pixelEntity);
        }
    }

    return splittedEntities;
};

std::vector<entt::entity> WeaponControlSystem::GetPhysicalBodiesInRaduis(
    const std::vector<entt::entity>& entities, const b2Vec2& center, float radius, std::optional<b2BodyType> bodyType)
{
    std::vector<entt::entity> result;

    for (auto& entity : entities)
    {
        auto physicsInfo = registry.get<PhysicsInfo>(entity);
        b2Body* body = physicsInfo.bodyRAII->GetBody();
        const b2Vec2& physicsPos = body->GetPosition();

        if (bodyType && body->GetType() != bodyType.value())
            continue;

        float distance = utils::distance(center, physicsPos);
        if (distance <= radius)
            result.push_back(entity);
    }

    return result;
};

std::vector<entt::entity> WeaponControlSystem::ExcludePlayersFromList(const std::vector<entt::entity>& entities)
{
    std::vector<entt::entity> result;
    for (auto& entity : entities)
    {
        if (!registry.any_of<PlayerInfo>(entity))
            result.push_back(entity);
    }
    return result;
};