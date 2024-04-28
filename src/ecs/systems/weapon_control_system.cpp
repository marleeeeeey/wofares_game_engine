#include "weapon_control_system.h"
#include <SDL_rect.h>
#include <box2d/b2_body.h>
#include <box2d/b2_math.h>
#include <ecs/components/physics_components.h>
#include <ecs/components/rendering_components.h>
#include <ecs/components/timer_components.h>
#include <ecs/components/weapon_components.h>
#include <entt/entity/fwd.hpp>
#include <my_cpp_utils/config.h>
#include <my_cpp_utils/math_utils.h>
#include <utils/box2d/box2d_body_tuner.h>
#include <utils/box2d/box2d_glm_conversions.h>
#include <utils/coordinates_transformer.h>
#include <utils/entt/entt_registry_requests.h>
#include <utils/entt/entt_registry_wrapper.h>
#include <utils/factories/box2d_body_creator.h>
#include <utils/logger.h>
#include <utils/sdl/sdl_colors.h>
#include <utils/sdl/sdl_texture_process.h>
#include <utils/systems/box2d_entt_contact_listener.h>

WeaponControlSystem::WeaponControlSystem(
    EnttRegistryWrapper& registryWrapper, Box2dEnttContactListener& contactListener, AudioSystem& audioSystem,
    ObjectsFactory& objectsFactory)
  : registryWrapper(registryWrapper), registry(registryWrapper.GetRegistry()),
    gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())), contactListener(contactListener),
    audioSystem(audioSystem), objectsFactory(objectsFactory), coordinatesTransformer(registry),
    physicsBodyTuner(registry)
{
    SubscribeToContactEvents();
}

void WeaponControlSystem::Update(float deltaTime)
{
    CheckTimerExplosionEntities();
    UpdateCollisionDisableTimerComponent(deltaTime);
    ProcessEntitiesQueues();
}

void WeaponControlSystem::SubscribeToContactEvents()
{
    contactListener.SubscribeContact(
        Box2dEnttContactListener::ContactType::Begin,
        [this](const Box2dEnttContactListener::ContactInfo& contactInfo)
        {
            for (const auto& explosionEntity : {contactInfo.entityA, contactInfo.entityB})
            {
                // If the entity contains the ExplosionOnContactComponent.
                if (!registry.all_of<ExplosionOnContactComponent, PhysicsComponent>(explosionEntity))
                    continue;

                // Check if any entity is ExplostionParticlesComponent. If so, then do not trigger the explosion.
                if (registry.any_of<ExplostionParticlesComponent>(contactInfo.entityA) ||
                    registry.any_of<ExplostionParticlesComponent>(contactInfo.entityB))
                    return;

                // If the entity contains the StickyComponent, check if it is sticked.
                bool shouldExplode = true;
                if (registry.all_of<StickyComponent>(explosionEntity))
                {
                    auto& stickFlagComponent = registry.get<StickyComponent>(explosionEntity);

                    if (!stickFlagComponent.isSticked)
                    {
                        // If it is in flight, then the explosion should not be triggered.
                        // Body should become static and explosion should be triggered on the next contact.
                        becomeStaticEntitiesQueue.insert(explosionEntity);
                        stickFlagComponent.isSticked = true;
                        shouldExplode = false;
                    }
                    else
                    {
                        // If it is sticked (installed), then the explosion should be triggered.
                        shouldExplode = true;
                    }
                }

                if (shouldExplode)
                {
                    // Calculate the contact point in the physics world.
                    auto physicsComponent = registry.get<PhysicsComponent>(explosionEntity);
                    std::optional<b2Vec2> contactPointPhysics;
                    if (contactInfo.contact->GetManifold()->pointCount > 0)
                    {
                        auto localPoint = contactInfo.contact->GetManifold()->points[0].localPoint;
                        contactPointPhysics = physicsComponent.bodyRAII->GetBody()->GetWorldPoint(localPoint);
                    }

                    AppendToExplosionQueue({explosionEntity, contactPointPhysics});
                }
            }
        });

    contactListener.SubscribeContact(
        Box2dEnttContactListener::ContactType::Begin,
        [this](const Box2dEnttContactListener::ContactInfo& contactInfo)
        {
            for (const auto& entity : {contactInfo.entityA, contactInfo.entityB})
            {
                // If the entity contains the CollisionDisableHitCountComponent.
                if (!registry.all_of<CollisionDisableHitCountComponent>(entity))
                    continue;

                UpdateCollisionDisableHitCountComponent(entity);
            }
        });
}

// Not allowed to update Box2D object in the contact listener. Because Box2D is in simulation step.
void WeaponControlSystem::AppendToExplosionQueue(const ExplosionEntityWithContactPoint& explosionEntityWithContactPoint)
{
    explosionEntitiesQueue[explosionEntityWithContactPoint.explosionEntity] = explosionEntityWithContactPoint;
}

void WeaponControlSystem::UpdateCollisionDisableHitCountComponent(entt::entity hitCountEntity)
{
    auto hitCount = registry.try_get<CollisionDisableHitCountComponent>(hitCountEntity);
    if (!hitCount)
        return;

    hitCount->hitCount--;

    if (hitCount->hitCount <= 0)
    {
        registry.remove<CollisionDisableHitCountComponent>(hitCountEntity);
        physicsBodyTuner.ApplyOption(hitCountEntity, {CollisionFlags::None, CollisionFlags::None});
    }
}

void WeaponControlSystem::CheckTimerExplosionEntities()
{
    auto entityWithTimers = registry.view<TimerComponent, ExplosionOnTimerComponent>();
    for (auto& timerExplosionEntity : entityWithTimers)
    {
        auto timerComponent = entityWithTimers.get<TimerComponent>(timerExplosionEntity);
        if (!timerComponent.isActivated)
            continue;

        DoExplosion({timerExplosionEntity, std::nullopt});
    }
}

void WeaponControlSystem::OnBazookaContactWithTile(
    [[maybe_unused]] entt::entity bazookaEntity, [[maybe_unused]] entt::entity tileEntity)
{
    MY_LOG(info, "Bazooka contact with tile");
}

void WeaponControlSystem::DoExplosion(const ExplosionEntityWithContactPoint& explosionEntityWithContactPoint)
{
    auto& explosionEntity = explosionEntityWithContactPoint.explosionEntity;

    auto damageComponent = registry.try_get<DamageComponent>(explosionEntity);
    auto physicsInfo = registry.try_get<PhysicsComponent>(explosionEntity);

    if (!damageComponent || !physicsInfo)
        return;

    // Get all physical bodies in the explosion radius.
    const b2Vec2 grenadePosPhysics =
        explosionEntityWithContactPoint.contactPointPhysics.value_or(physicsInfo->bodyRAII->GetBody()->GetPosition());

    float radiusCoef = 1.2f; // TODO0: hack. Need to calculate it based on the texture size. Because position is
                             // calculated from the center of the texture.

    std::vector<entt::entity> allOriginalBodiesInRadius = request::FindEntitiesWithAllComponentsInRadius(
        registry, grenadePosPhysics, damageComponent->radius * radiusCoef);
    MY_LOG(debug, "[DoExplosion] FindEntitiesInRadius count {}", allOriginalBodiesInRadius.size());

    auto destructibleOriginalBodies =
        request::GetEntitiesWithAllComponents<DestructibleComponent>(registry, allOriginalBodiesInRadius);
    destructibleOriginalBodies =
        request::RemoveEntitiesWithAllComponents<ExplostionParticlesComponent>(registry, destructibleOriginalBodies);
    MY_LOG(debug, "[DoExplosion] Getting destructible objects. Count {}", destructibleOriginalBodies.size());

    // Split original objects to micro objects.
    auto& cellSizeForMicroDistruction = utils::GetConfig<int, "WeaponControlSystem.cellSizeForMicroDistruction">();
    SDL_Point cellSize = {cellSizeForMicroDistruction, cellSizeForMicroDistruction};
    auto newMicroBodies = objectsFactory.SpawnSplittedPhysicalEnteties(destructibleOriginalBodies, cellSize);
    MY_LOG(debug, "[DoExplosion] Spawn micro splittedEntities count {}", newMicroBodies.size());

    // Get micro objects in the explosion radius.
    auto newMicroBodiesToDestroy = request::FilterEntitiesWithAllComponentsInRadius(
        registry, newMicroBodies, grenadePosPhysics, damageComponent->radius);

    // Destroy micro objects in the explosion radius.
    MY_LOG(debug, "[DoExplosion] Destroing {} micro objects", newMicroBodiesToDestroy.size());
    for (auto& entity : newMicroBodiesToDestroy)
        registryWrapper.Destroy(entity);

    if (utils::GetConfig<bool, "WeaponControlSystem.keepTilesAliveOnExplosion">())
    {
        // Apply force to micro objects from the explosion center.
        for (auto& entity : destructibleOriginalBodies)
        {
            // If entity contains PixeledTileComponent, then remove it.
            // Need to prevent dust particles from the tile. Save CPU time.
            if (registry.all_of<PixeledTileComponent>(entity))
            {
                registryWrapper.Destroy(entity);
                continue;
            }

            physicsBodyTuner.ApplyOption(entity, Box2dBodyOptions::MovementPolicy::Box2dPhysics);
            // How to read: "I have own collision category Default and I want collide with Default".
            physicsBodyTuner.ApplyOption(entity, {CollisionFlags::Default, CollisionFlags::Default});
            registry.emplace_or_replace<ExplostionParticlesComponent>(entity);

            auto& physicsComponent = registry.get<PhysicsComponent>(entity);
            auto body = physicsComponent.bodyRAII->GetBody();
            auto bodyPos = body->GetPosition();

            // Apply force to the body.
            auto vec = bodyPos - grenadePosPhysics;
            vec.Normalize();
            float force = damageComponent->force;
            body->ApplyLinearImpulseToCenter(force * vec, true);
        }
    }
    else
    {
        // Destroy original objects.
        for (auto& entity : destructibleOriginalBodies)
        {
            registryWrapper.Destroy(entity);
        }
    }

    if (utils::GetConfig<bool, "WeaponControlSystem.createSyntheticExplosionFragments">())
    {
        glm::vec2 fragmentsCenterWorld = coordinatesTransformer.PhysicsToWorld(grenadePosPhysics);
        float fragmentRadiusWorld = coordinatesTransformer.PhysicsToWorld(damageComponent->radius);
        objectsFactory.SpawnFragmentsAfterExplosion(fragmentsCenterWorld, fragmentRadiusWorld);
    }

    // Destroy the explosion entity.
    registryWrapper.Destroy(explosionEntity);

    // Play explosion sound.
    audioSystem.PlaySoundEffect("explosion");
}

void WeaponControlSystem::ProcessEntitiesQueues()
{
    for (auto entity : becomeStaticEntitiesQueue)
    {
        physicsBodyTuner.ApplyOption(entity, Box2dBodyOptions::MovementPolicy::Manual);
    }

    for (const auto& [entity, entotyWithCollisionPoint] : explosionEntitiesQueue)
    {
        // Here was a bug. If the entity is already in the becomeStaticEntitiesQueue, then the explosion should not be
        // triggered. This means that the entity just hit the wall and should become static. Explosion should be on the
        // next contact.
        if (!becomeStaticEntitiesQueue.contains(entotyWithCollisionPoint.explosionEntity))
        {
            DoExplosion(entotyWithCollisionPoint);
        }
    }

    explosionEntitiesQueue.clear();
    becomeStaticEntitiesQueue.clear();
}

void WeaponControlSystem::UpdateCollisionDisableTimerComponent(float deltaTime)
{
    auto collisionDisableTimers = registry.view<CollisionDisableTimerComponent>();
    for (auto entity : collisionDisableTimers)
    {
        auto& collisionDisableTimer = collisionDisableTimers.get<CollisionDisableTimerComponent>(entity);
        collisionDisableTimer.timeToDisableCollision -= deltaTime;

        if (collisionDisableTimer.timeToDisableCollision <= 0.0f)
        {
            registry.remove<CollisionDisableTimerComponent>(entity);
            physicsBodyTuner.ApplyOption(entity, {CollisionFlags::None, CollisionFlags::None});
        }
    }
}