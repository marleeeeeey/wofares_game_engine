#pragma once
#include "SDL_render.h"
#include <entt/entt.hpp>

void LoadMap(entt::registry& registry, SDL_Renderer* renderer, const std::string& filename);
