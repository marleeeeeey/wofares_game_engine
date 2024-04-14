#pragma once
#include <SDL.h>
#include <glm/glm.hpp>

void DrawCross(SDL_Renderer* renderer, int centerX, int centerY, int size, const SDL_Color& color);

void DrawCross(SDL_Renderer* renderer, const glm::vec2& center, int size, const SDL_Color& color);
