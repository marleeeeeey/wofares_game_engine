#pragma once
#include "utils/sdl_texture_process.h"
#include <entt/entt.hpp>
#include <utils/entt_registry_wrapper.h>
#include <utils/factories/box2d_body_creator.h>
#include <utils/game_options.h>
#include <utils/resources/resource_manager.h>

class ObjectsFactory
{
    EnttRegistryWrapper& registryWrapper;
    entt::registry& registry;
    ResourceManager& resourceManager;
    GameOptions& gameState;
    std::shared_ptr<b2World> physicsWorld;
    Box2dBodyCreator box2dBodyCreator;
public:
    ObjectsFactory(EnttRegistryWrapper& registryWrapper, ResourceManager& resourceManager);
    entt::entity CreateTile(
        glm::vec2 posWorld, float sizeWorld, const TextureRect& textureRect, const std::string& name = "Tile");
    entt::entity CreatePlayer(const glm::vec2& playerSdlWorldPos);
    entt::entity SpawnFlyingEntity(
        const glm::vec2& posWorld, const glm::vec2& sizeWorld, const glm::vec2& forceDirection, float force);
public: // Explosions.
    // Split physical entities into smaller ones. Return new entities. Used for explosion effect.
    std::vector<entt::entity> SpawnSplittedPhysicalEnteties(
        const std::vector<entt::entity>& entities, SDL_Point cellSize);
public: // Fragments.
    entt::entity CreateFragmentAfterExplosion(const glm::vec2& sdlWorldPos);
    std::vector<entt::entity> SpawnFragmentsAfterExplosion(glm::vec2 centerWorld, float radiusWorld);
private: // Animation.
    AnimationInfo CreateAnimationInfo(
        const std::string& animationName, const std::string& tagName, ResourceManager::TagProps tagProps);
};