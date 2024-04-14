#pragma once
#include "ecs/components/rendering_components.h"
#include <ecs/components/animation_components.h>
#include <entt/entt.hpp>
#include <utils/box2d/box2d_body_tuner.h>
#include <utils/coordinates_transformer.h>
#include <utils/entt/entt_registry_wrapper.h>
#include <utils/factories/box2d_body_creator.h>
#include <utils/game_options.h>
#include <utils/resources/resource_manager.h>
#include <utils/sdl/sdl_texture_process.h>

// This class is responsible for creating objects in the registry.
// There shouldn't be any place in the code where we create objects directly in the registry.
// Try searching for "registry.emplace" in the code. If you find it, it's a bug.
class ObjectsFactory
{
    EnttRegistryWrapper& registryWrapper;
    entt::registry& registry;
    ResourceManager& resourceManager;
    GameOptions& gameState;
    std::shared_ptr<b2World> physicsWorld;
    Box2dBodyCreator box2dBodyCreator;
    CoordinatesTransformer coordinatesTransformer;
    Box2dBodyTuner bodyTuner;
public:
    ObjectsFactory(EnttRegistryWrapper& registryWrapper, ResourceManager& resourceManager);
public: // Main game objects.
    struct SpawnTileOption
    {
        enum class DesctructibleOption
        {
            Destructible,
            NoDestructible,
        } destructibleOption = DesctructibleOption::Destructible;

        ZOrderingType zOrderingType = ZOrderingType::Terrain;
    };
    entt::entity SpawnTile(
        glm::vec2 posWorld, float sizeWorld, const TextureRect& textureRect, SpawnTileOption tileOptions,
        const std::string& name = "Tile");
    entt::entity SpawnPlayer(const glm::vec2& posWorld, const std::string& debugName);
    entt::entity SpawnBullet(
        entt::entity playerEntity, float initialBulletSpeed, Box2dBodyOptions::AnglePolicy anglePolicy);
    entt::entity SpawnBuildingBlock(glm::vec2 posWorld);
    entt::entity SpawnPortal(const glm::vec2& posWorld, const std::string& debugName);
public: // Explosions.
    // Split physical entities into smaller ones. Return new entities. Used for explosion effect.
    std::vector<entt::entity> SpawnSplittedPhysicalEnteties(
        const std::vector<entt::entity>& entities, SDL_Point cellSizeWorld);
    entt::entity SpawnFragmentAfterExplosion(const glm::vec2& posWorld);
    std::vector<entt::entity> SpawnFragmentsAfterExplosion(glm::vec2 centerWorld, float radiusWorld);
private: // Helpers
    AnimationComponent CreateAnimationInfo(
        const std::string& animationName, const std::string& tagName, ResourceManager::TagProps tagProps);
    entt::entity SpawnFlyingEntity(
        const glm::vec2& posWorld, const glm::vec2& sizeWorld, const glm::vec2& forceDirection, float force,
        Box2dBodyOptions::AnglePolicy anglePolicy);
};