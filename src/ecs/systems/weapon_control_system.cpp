#include "weapon_control_system.h"
#include <SDL_rect.h>
#include <box2d/b2_body.h>
#include <box2d/b2_math.h>
#include <ecs/components/physics_components.h>
#include <ecs/components/weapon_components.h>
#include <entt/entity/fwd.hpp>
#include <my_cpp_utils/config.h>
#include <my_cpp_utils/logger.h>
#include <my_cpp_utils/math_utils.h>
#include <utils/box2d_body_tuner.h>
#include <utils/coordinates_transformer.h>
#include <utils/entt_collect_objects.h>
#include <utils/entt_registry_wrapper.h>
#include <utils/factories/box2d_body_creator.h>
#include <utils/glm_box2d_conversions.h>
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
    UpdateTimerExplosionComponents(deltaTime);
    UpdateContactExplosionComponentTimer(deltaTime);
    UpdateCollisionDisableTimerComponent(deltaTime);
    ProcessExplosionEntitiesQueue();
}

void WeaponControlSystem::SubscribeToContactEvents()
{
    contactListener.SubscribeContact(
        Box2dEnttContactListener::ContactType::Begin,
        [this](const Box2dEnttContactListener::ContactInfo& contactInfo)
        {
            for (const auto& explosionEntity : {contactInfo.entityA, contactInfo.entityB})
            {
                // If the entity contains the ContactExplosionComponent.
                if (!registry.all_of<ContactExplosionComponent, PhysicsComponent>(explosionEntity))
                    continue;

                auto physicsComponent = registry.get<PhysicsComponent>(explosionEntity);

                std::optional<b2Vec2> contactPointPhysics;
                if (contactInfo.contact->GetManifold()->pointCount > 0)
                {
                    auto localPoint = contactInfo.contact->GetManifold()->points[0].localPoint;
                    contactPointPhysics = physicsComponent.bodyRAII->GetBody()->GetWorldPoint(localPoint);
                }

                OnContactWithExplosionComponent({explosionEntity, contactPointPhysics});
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
    auto& contactExplosion = registry.get<ContactExplosionComponent>(explosionEntityWithContactPoint.explosionEntity);
    if (contactExplosion.spawnSafeTime <= 0.0f)
    {
        explosionEntities.push(explosionEntityWithContactPoint);
    }
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

void WeaponControlSystem::UpdateTimerExplosionComponents(float deltaTime)
{
    auto timersView = registry.view<TimerExplosionComponent>();
    for (auto& timerEntity : timersView)
    {
        auto& timerExplosion = timersView.get<TimerExplosionComponent>(timerEntity);
        timerExplosion.timeToExplode -= deltaTime;

        if (timerExplosion.timeToExplode <= 0.0f)
        {
            DoExplosion({timerEntity, std::nullopt});
        }
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

    auto explosionImpact = registry.try_get<ExplosionImpactComponent>(explosionEntity);
    auto physicsInfo = registry.try_get<PhysicsComponent>(explosionEntity);

    if (!explosionImpact || !physicsInfo)
        return;

    // Get all physical bodies in the explosion radius.
    const b2Vec2 grenadePosPhysics =
        explosionEntityWithContactPoint.contactPointPhysics.value_or(physicsInfo->bodyRAII->GetBody()->GetPosition());

    float radiusCoef = 1.2f; // TODO0: hack. Need to calculate it based on the texture size. Because position is
                             // calculated from the center of the texture.
    auto staticOriginalBodies = collectObjects.GetPhysicalBodiesInRaduis(
        grenadePosPhysics, explosionImpact->radius * radiusCoef, b2_staticBody);

    // Split original objects to micro objects.
    auto& cellSizeForMicroDistruction = utils::GetConfig<int, "WeaponControlSystem.cellSizeForMicroDistruction">();
    SDL_Point cellSize = {cellSizeForMicroDistruction, cellSizeForMicroDistruction};
    auto splittedEntities = objectsFactory.SpawnSplittedPhysicalEnteties(staticOriginalBodies, cellSize);

    // Destroy micro objects in the explosion radius.
    auto staticMicroBodiesToDestroy = collectObjects.GetPhysicalBodiesInRaduis(
        splittedEntities, grenadePosPhysics, explosionImpact->radius, b2_staticBody);
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
        float fragmentRadiusWorld = coordinatesTransformer.PhysicsToWorld(explosionImpact->radius);
        objectsFactory.SpawnFragmentsAfterExplosion(fragmentsCenterWorld, fragmentRadiusWorld);
    }

    // Destroy the explosion entity.
    registryWrapper.Destroy(explosionEntity);

    // Play explosion sound.
    audioSystem.PlaySoundEffect("explosion");
}

void WeaponControlSystem::ProcessExplosionEntitiesQueue()
{
    while (!explosionEntities.empty())
    {
        auto entity = explosionEntities.front();
        DoExplosion(entity);
        explosionEntities.pop();
    }
}

void WeaponControlSystem::UpdateContactExplosionComponentTimer(float deltaTime)
{
    auto contactExplosionsView = registry.view<ContactExplosionComponent>();
    for (auto& entity : contactExplosionsView)
    {
        auto& contactExplosion = contactExplosionsView.get<ContactExplosionComponent>(entity);
        contactExplosion.spawnSafeTime -= deltaTime;
    }
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