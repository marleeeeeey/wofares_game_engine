#pragma once
#include <SDL.h>
#include <glm/vec2.hpp>
#include <vector>

enum class FillOption
{
    Filled,
    Outlined
};

int polygonRGBA(
    SDL_Renderer* renderer, std::vector<glm::vec2> vertices, SDL_Color color,
    FillOption fillOption = FillOption::Outlined);

int circleRGBA(
    SDL_Renderer* renderer, glm::vec2 center, float radius, SDL_Color color,
    FillOption fillOption = FillOption::Outlined);
