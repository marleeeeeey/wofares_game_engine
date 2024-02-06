#pragma once
#include <SDL2/SDL.h>
#include <ecs/components/all_components.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>

void RenderHUDSystem(entt::registry& registry, SDL_Renderer* renderer);

void DrawGridSystem(SDL_Renderer* renderer, const GameState& gameState);
