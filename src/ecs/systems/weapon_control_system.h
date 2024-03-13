#pragma once
#include "utils/coordinates_transformer.h"
#include "utils/factories/objects_factory.h"
#include "utils/resources/resource_manager.h"
#include <entt/entt.hpp>
#include <optional>
#include <queue>
#include <utils/entt_registry_wrapper.h>
#include <utils/game_options.h>
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
    float deltaTime;
    std::queue<entt::entity> contactedEntities;
    CoordinatesTransformer coordinatesTransformer;
public:
    WeaponControlSystem(
        EnttRegistryWrapper& registryWrapper, Box2dEnttContactListener& contactListener, AudioSystem& audioSystem,
        ObjectsFactory& objectsFactory);
    void Update(float deltaTime);
private:
    void SubscribeToContactEvents();
    void OnContactWithExplosionComponent(entt::entity explosionEntity, entt::entity contactedEntity);
private:
    void UpdateTimerExplosionComponents();
    void UpdateContactExplosionComponentTimer();
    void UpdateCollisionDisableHitCountComponent();
    void ProcessExplosionEntitiesQueue();
    void OnBazookaContactWithTile(entt::entity bazookaEntity, entt::entity tileEntity);
    void DoExplosion(entt::entity explosionEntity);
private: // TODO3: functions to move to shared code like utils.
    std::vector<entt::entity> GetPhysicalBodiesInRaduis(
        const b2Vec2& center, float radius, std::optional<b2BodyType> bodyType);
    std::vector<entt::entity> GetPhysicalBodiesInRaduis(
        const std::vector<entt::entity>& entities, const b2Vec2& center, float physicalRadius,
        std::optional<b2BodyType> bodyType);
    std::vector<entt::entity> AddAndReturnSplittedPhysicalEntetiesToWorld(
        const std::vector<entt::entity>& entities, SDL_Point cellSize);
    std::vector<entt::entity> ExcludePlayersFromList(const std::vector<entt::entity>& entities);
};