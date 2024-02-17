#include <ecs/systems/details/map_loader.h>

void UnloadMap(entt::registry& registry)
{
    auto& gameState = registry.get<GameState>(registry.view<GameState>().front());
    gameState.levelOptions.levelBox2dBounds = {};

    // Remove all entities that have a RenderingInfo component.
    for (auto entity : registry.view<RenderingInfo>())
        registry.destroy(entity);

    // Remove all entities that have a PhysicalBody component.
    for (auto entity : registry.view<PhysicsInfo>())
        registry.destroy(entity);

    if (Box2dObjectRAII::GetBodyCounter() != 0)
        MY_LOG_FMT(warn, "There are still {} Box2D bodies in the memory", Box2dObjectRAII::GetBodyCounter());
    else
        MY_LOG(debug, "All Box2D bodies were destroyed");
}

void LoadMap(entt::registry& registry, SDL_Renderer* renderer, const std::string& filename)
{
    MapLoader(filename, registry, renderer);
}
