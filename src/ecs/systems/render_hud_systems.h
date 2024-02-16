#pragma once
#include <SDL2/SDL.h>
#include <ecs/components/game_components.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>

void RenderHUDSystem(entt::registry& registry, SDL_Renderer* renderer);

void DrawGridSystem(entt::registry& registry, SDL_Renderer* renderer);
