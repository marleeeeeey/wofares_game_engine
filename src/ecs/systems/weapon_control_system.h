#pragma once
#include "utils/collect_objects.h"
#include "utils/coordinates_transformer.h"
#include "utils/factories/objects_factory.h"
#include <entt/entt.hpp>
#include <queue>
#include <utils/entt_registry_wrapper.h>
#include <utils/game_options.h>
#include <utils/physics_body_tuner.h>
#include <utils/systems/audio_system.h>
#include <utils/systems/box2d_entt_contact_listener.h>

class WeaponControlSystem
{
    EnttRegistryWrapper& registryWrapper;
    entt::registry& registry;
    GameOptions& gameState;
    Box2dEnttContactListener& contactListener;
    AudioSystem& audioSystem;
    ObjectsFactory& objectsFactory;
    std::queue<entt::entity> contactedEntities;
    CoordinatesTransformer coordinatesTransformer;
    CollectObjects collectObjects;
    PhysicsBodyTuner PhysicsBodyTuner;
public:
    WeaponControlSystem(
        EnttRegistryWrapper& registryWrapper, Box2dEnttContactListener& contactListener, AudioSystem& audioSystem,
        ObjectsFactory& objectsFactory);
    void Update(float deltaTime);
private:
    void SubscribeToContactEvents();
    void OnContactWithExplosionComponent(entt::entity explosionEntity, entt::entity contactedEntity);
private:
    void UpdateTimerExplosionComponents(float deltaTime);
    void UpdateContactExplosionComponentTimer(float deltaTime);
    void UpdateCollisionDisableTimerComponent(float deltaTime);
    void UpdateCollisionDisableHitCountComponent(entt::entity hitCountEntity);
    void ProcessExplosionEntitiesQueue();
    void OnBazookaContactWithTile(entt::entity bazookaEntity, entt::entity tileEntity);
    void DoExplosion(entt::entity explosionEntity);
};