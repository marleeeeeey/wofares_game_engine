#include "weapon_control_system.h"
#include "my_cpp_utils/logger.h"
#include <SDL_rect.h>
#include <box2d/b2_body.h>
#include <box2d/b2_math.h>
#include <ecs/components/physics_components.h>
#include <ecs/components/timer_components.h>
#include <ecs/components/weapon_components.h>
#include <entt/entity/fwd.hpp>
#include <my_cpp_utils/config.h>
#include <my_cpp_utils/math_utils.h>
#include <utils/box2d_body_tuner.h>
#include <utils/coordinates_transformer.h>
#include <utils/entt_collect_objects.h>
#include <utils/entt_registry_wrapper.h>
#include <utils/factories/box2d_body_creator.h>
#include <utils/glm_box2d_conversions.h>
#include <utils/logger.h>
#include <utils/sdl_colors.h>
#include <utils/sdl_texture_process.h>
#include <utils/systems/box2d_entt_contact_listener.h>

WeaponControlSystem::WeaponControlSystem(
    EnttRegistryWrapper& registryWrapper, Box2dEnttContactListener& contactListener, AudioSystem& audioSystem,
    ObjectsFactory& objectsFactory)
  : registryWrapper(registryWrapper), registry(registryWrapper.GetRegistry()),
    gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())), contactListener(contactListener),
    audioSystem(audioSystem), objectsFactory(objectsFactory), coordinatesTransformer(registry),
    collectObjects(registry), physicsBodyTuner(registry)
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

                    OnContactWithExplosionComponent({explosionEntity, contactPointPhysics});
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
void WeaponControlSystem::OnContactWithExplosionComponent(
    const ExplosionEntityWithContactPoint& explosionEntityWithContactPoint)
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
        physicsBodyTuner.ApplyOption(hitCountEntity, Box2dBodyOptions::CollisionPolicy::NoCollision);
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
    std::vector<entt::entity> staticOriginalBodies = collectObjects.GetPhysicalBodiesInRaduis(
        grenadePosPhysics, damageComponent->radius * radiusCoef, b2_staticBody);

    // Remove bodies with flag Box2dBodyOptions::DestructionPolicy::Indestructible
    staticOriginalBodies.erase(
        std::remove_if(
            staticOriginalBodies.begin(), staticOriginalBodies.end(),
            [this](entt::entity entity)
            {
                auto& physicsComponent = registry.get<PhysicsComponent>(entity);
                return physicsComponent.options.destructionPolicy ==
                    Box2dBodyOptions::DestructionPolicy::Indestructible;
            }),
        staticOriginalBodies.end());

    // Split original objects to micro objects.
    auto& cellSizeForMicroDistruction = utils::GetConfig<int, "WeaponControlSystem.cellSizeForMicroDistruction">();
    SDL_Point cellSize = {cellSizeForMicroDistruction, cellSizeForMicroDistruction};
    auto splittedEntities = objectsFactory.SpawnSplittedPhysicalEnteties(staticOriginalBodies, cellSize);

    // Destroy micro objects in the explosion radius.
    auto staticMicroBodiesToDestroy = collectObjects.GetPhysicalBodiesInRaduis(
        splittedEntities, grenadePosPhysics, damageComponent->radius, b2_staticBody);
    for (auto& entity : staticMicroBodiesToDestroy)
        registryWrapper.Destroy(entity);

    // Destroy original objects.
    for (auto& entity : staticOriginalBodies)
    {
        registryWrapper.Destroy(entity);
    }

    if (utils::GetConfig<bool, "WeaponControlSystem.createExplosionFragments">())
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
        MY_LOG(info, "Entity become static: {0}", entity);
        physicsBodyTuner.ApplyOption(entity, Box2dBodyOptions::DynamicOption::Static);
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
            physicsBodyTuner.ApplyOption(entity, Box2dBodyOptions::CollisionPolicy::NoCollision);
        }
    }
}