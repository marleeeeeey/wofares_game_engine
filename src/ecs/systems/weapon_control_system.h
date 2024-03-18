#pragma once
#include <entt/entt.hpp>
#include <queue>
#include <utils/box2d_body_tuner.h>
#include <utils/coordinates_transformer.h>
#include <utils/entt_collect_objects.h>
#include <utils/entt_registry_wrapper.h>
#include <utils/factories/objects_factory.h>
#include <utils/game_options.h>
#include <utils/systems/audio_system.h>
#include <utils/systems/box2d_entt_contact_listener.h>

class WeaponControlSystem
{
    struct ExplosionEntityWithContactPoint
    {
        entt::entity explosionEntity;
        std::optional<b2Vec2> contactPointPhysics;
    };

    EnttRegistryWrapper& registryWrapper;
    entt::registry& registry;
    GameOptions& gameState;
    Box2dEnttContactListener& contactListener;
    AudioSystem& audioSystem;
    ObjectsFactory& objectsFactory;
    std::queue<ExplosionEntityWithContactPoint> explosionEntities;
    CoordinatesTransformer coordinatesTransformer;
    EnttCollectObjects collectObjects;
    Box2dBodyTuner physicsBodyTuner;
public:
    WeaponControlSystem(
        EnttRegistryWrapper& registryWrapper, Box2dEnttContactListener& contactListener, AudioSystem& audioSystem,
        ObjectsFactory& objectsFactory);
    void Update(float deltaTime);
private:
    void SubscribeToContactEvents();
    void OnContactWithExplosionComponent(const ExplosionEntityWithContactPoint& explosionEntityWithContactPoint);
private:
    void UpdateTimerExplosionComponents(float deltaTime);
    void UpdateContactExplosionComponentTimer(float deltaTime);
    void UpdateCollisionDisableTimerComponent(float deltaTime);
    void UpdateCollisionDisableHitCountComponent(entt::entity hitCountEntity);
    void ProcessExplosionEntitiesQueue();
    void OnBazookaContactWithTile(entt::entity bazookaEntity, entt::entity tileEntity);
    void DoExplosion(const ExplosionEntityWithContactPoint& explosionEntityWithContactPoint);
};