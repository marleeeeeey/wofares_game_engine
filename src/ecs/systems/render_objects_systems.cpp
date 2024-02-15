#include "render_objects_systems.h"
#include <ecs/components/all_components.h>
#include <ecs/systems/details/game_objects_renderer.h>
#include <utils/glm_box2d_conversions.h>
#include <utils/sdl_colors.h>

void RenderSystem(entt::registry& registry, SDL_Renderer* renderer)
{
    GameObjectsRenderer(registry, renderer);
}
