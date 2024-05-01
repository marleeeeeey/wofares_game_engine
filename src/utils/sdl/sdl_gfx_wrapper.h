#pragma once
#include <SDL.h>
#include <glm/vec2.hpp>
#include <vector>

enum class FillOption
{
    Filled,
    Outlined
};

int DrawPoligon(
    SDL_Renderer* renderer, std::vector<glm::vec2> vertices, SDL_Color color,
    FillOption fillOption = FillOption::Outlined);

int DrawCircle(
    SDL_Renderer* renderer, glm::vec2 center, float radius, SDL_Color color,
    FillOption fillOption = FillOption::Outlined);
