#pragma once
#include <SDL_render.h>
#include <entt/entt.hpp>

void UnloadMap(entt::registry& registry);

void LoadMap(entt::registry& registry, SDL_Renderer* renderer, const std::string& filename);
