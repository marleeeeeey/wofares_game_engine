#pragma once
#include "utils/factories/base_objects_factory.h"
#include <ecs/components/animation_components.h>
#include <ecs/components/rendering_components.h>
#include <entt/entt.hpp>
#include <utils/box2d/box2d_body_tuner.h>
#include <utils/coordinates_transformer.h>
#include <utils/entt/entt_registry_wrapper.h>
#include <utils/factories/box2d_body_creator.h>
#include <utils/factories/components_factory.h>
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
    GameOptions& gameState;
    Box2dBodyCreator box2dBodyCreator;
    CoordinatesTransformer coordinatesTransformer;
    Box2dBodyTuner bodyTuner;
    ComponentsFactory& componentsFactory;
    BaseObjectsFactory& baseObjectsFactory;
public:
    ObjectsFactory(
        EnttRegistryWrapper& registryWrapper, ComponentsFactory& componentsFactory,
        BaseObjectsFactory& baseObjectsFactory);
public: // Main game objects.
    entt::entity SpawnPlayer(const glm::vec2& posWorld, const std::string& debugName);
    entt::entity SpawnBullet(
        entt::entity playerEntity, float initialBulletSpeed, Box2dBodyOptions::AnglePolicy anglePolicy);
    entt::entity SpawnBuildingBlock(glm::vec2 posWorld);
    entt::entity SpawnPortal(const glm::vec2& posWorld, const std::string& debugName);
    entt::entity SpawnTurret(const glm::vec2& posWorld, const std::string& debugName);
};