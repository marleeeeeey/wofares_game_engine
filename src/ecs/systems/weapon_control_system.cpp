#include "weapon_control_system.h"
#include "my_common_cpp_utils/config.h"
#include "utils/coordinates_transformer.h"
#include "utils/physics_methods.h"
#include <SDL_rect.h>
#include <box2d/b2_body.h>
#include <box2d/b2_math.h>
#include <cassert>
#include <cstddef>
#include <ecs/components/game_components.h>
#include <entt/entity/fwd.hpp>
#include <my_common_cpp_utils/logger.h>
#include <my_common_cpp_utils/math_utils.h>
#include <utils/box2d_helpers.h>
#include <utils/entt_registry_wrapper.h>
#include <utils/factories/box2d_body_creator.h>
#include <utils/glm_box2d_conversions.h>
#include <utils/sdl_colors.h>
#include <utils/sdl_texture_process.h>
#include <utils/systems/box2d_entt_contact_listener.h>
#include <vector>

WeaponControlSystem::WeaponControlSystem(
    EnttRegistryWrapper& registryWrapper, Box2dEnttContactListener& contactListener, AudioSystem& audioSystem,
    ObjectsFactory& objectsFactory)
  : registryWrapper(registryWrapper), registry(registryWrapper.GetRegistry()),
    gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())), contactListener(contactListener),
    audioSystem(audioSystem), objectsFactory(objectsFactory), coordinatesTransformer(registry)
{
    SubscribeToContactEvents();
}

void WeaponControlSystem::Update(float deltaTime)
{
    this->deltaTime = deltaTime;
    UpdateTimerExplosionComponents();
    UpdateContactExplosionComponentTimer();
    ProcessExplosionEntitiesQueue();
}

void WeaponControlSystem::SubscribeToContactEvents()
{
    contactListener.SubscribeContact(
        Box2dEnttContactListener::ContactType::Begin,
        [this](entt::entity entityA, entt::entity entityB)
        {
            for (const auto& explosionEntity : {entityA, entityB})
            {
                // If the entity contains the ContactExplosionComponent.
                if (!registry.all_of<ContactExplosionComponent>(explosionEntity))
                    continue;

                // Other entity is the entity that was contacted with the explosion.
                auto contactedEntity = entityA == explosionEntity ? entityB : entityA;

                OnContactWithExplosionComponent(explosionEntity, contactedEntity);
            }
        });
}

// Not allowed to update Box2D object in the contact listener. Because Box2D is in simulation step.
void WeaponControlSystem::OnContactWithExplosionComponent(entt::entity explosionEntity, entt::entity contactedEntity)
{
    auto& contactExplosion = registry.get<ContactExplosionComponent>(explosionEntity);
    if (contactExplosion.spawnSafeTime <= 0.0f)
    {
        contactedEntities.push(explosionEntity);
    }
};

void WeaponControlSystem::UpdateTimerExplosionComponents()
{
    auto timersView = registry.view<TimerExplosionComponent>();
    for (auto& timerEntity : timersView)
    {
        auto& timerExplosion = timersView.get<TimerExplosionComponent>(timerEntity);
        timerExplosion.timeToExplode -= deltaTime;

        if (timerExplosion.timeToExplode <= 0.0f)
        {
            DoExplosion(timerEntity);
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

void WeaponControlSystem::OnBazookaContactWithTile(entt::entity bazookaEntity, entt::entity tileEntity)
{
    MY_LOG(info, "Bazooka contact with tile");
};

void WeaponControlSystem::DoExplosion(entt::entity explosionEntity)
{
    auto explosionImpact = registry.try_get<ExplosionImpactComponent>(explosionEntity);
    auto physicsInfo = registry.try_get<PhysicsInfo>(explosionEntity);

    if (!explosionImpact || !physicsInfo)
        return;

    // Get all physical bodies in the explosion radius.
    const b2Vec2& grenadePhysicsPos = physicsInfo->bodyRAII->GetBody()->GetPosition();
    float radiusCoef = 1.2f; // TODO0: hack. Need to calculate it based on the texture size. Because position is
                             // calculated from the center of the texture.
    auto staticOriginalBodies =
        GetPhysicalBodiesInRaduis(grenadePhysicsPos, explosionImpact->radius * radiusCoef, b2_staticBody);

    // Split original objects to micro objects.
    auto& cellSizeForMicroDistruction = utils::GetConfig<int, "weaponControlSystem.cellSizeForMicroDistruction">();
    SDL_Point cellSize = {cellSizeForMicroDistruction, cellSizeForMicroDistruction};
    auto splittedEntities = AddAndReturnSplittedPhysicalEntetiesToWorld(staticOriginalBodies, cellSize);

    // Destroy micro objects in the explosion radius.
    auto staticMicroBodiesToDestroy =
        GetPhysicalBodiesInRaduis(splittedEntities, grenadePhysicsPos, explosionImpact->radius, b2_staticBody);
    for (auto& entity : staticMicroBodiesToDestroy)
        registryWrapper.Destroy(entity);

    // Destroy original objects.
    for (auto& entity : staticOriginalBodies)
    {
        registryWrapper.Destroy(entity);
    }

    {
        glm::vec2 centerSdl = coordinatesTransformer.PhysicsToWorld(grenadePhysicsPos);
        float radiusSdl = coordinatesTransformer.PhysicsToWorld(explosionImpact->radius);
        size_t fragmentsCount = static_cast<size_t>(radiusSdl * 0.2f * utils::Random<float>(1, 1.2));
        std::vector<entt::entity> fragments;
        for (size_t i = 0; i < fragmentsCount; ++i)
        {
            auto fragmentRandomPos = utils::GetRandomCoordinateAround(centerSdl, radiusSdl);
            auto fragmentEntity = objectsFactory.CreateFragmentAfterExplosion(fragmentRandomPos);
            fragments.push_back(fragmentEntity);
        }
        PhysicsMethods physicsMethods(registry);
        physicsMethods.ApplyForceToPhysicalBodies(fragments, grenadePhysicsPos, 500.0f);
    }

    // Destroy the explosion entity.
    registryWrapper.Destroy(explosionEntity);

    // Play explosion sound.
    audioSystem.PlaySoundEffect("explosion");
};

void WeaponControlSystem::ProcessExplosionEntitiesQueue()
{
    while (!contactedEntities.empty())
    {
        auto entity = contactedEntities.front();
        DoExplosion(entity);
        contactedEntities.pop();
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
        if (originalObjRenderingInfo.textureRect.w <= cellSize.x ||
            originalObjRenderingInfo.textureRect.h <= cellSize.y)
            continue;

        auto originalRectPosInTexture =
            glm::vec2(originalObjRenderingInfo.textureRect.x, originalObjRenderingInfo.textureRect.y);

        auto textureRects = DivideRectByCellSize(originalObjRenderingInfo.textureRect, cellSize);
        for (auto& rect : textureRects)
        {
            // Caclulate position of the pixel in the world.
            auto pixelRectPosInTexture = glm::vec2(rect.x, rect.y);
            glm::vec2 pixelWorldPosition = originalObjWorldPos + (pixelRectPosInTexture - originalRectPosInTexture) -
                cellSizeGlm - glm::vec2{1, 1}; // TODO1: here is a hack with {1, 1}.

            assert(cellSize.x == cellSize.y);
            auto pixelEntity = objectsFactory.CreateTile(
                pixelWorldPosition, cellSize.x, TextureRect{originalObjRenderingInfo.texturePtr, rect}, "PixeledTile");

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

        float distance = utils::CaclDistance(center, physicsPos);
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