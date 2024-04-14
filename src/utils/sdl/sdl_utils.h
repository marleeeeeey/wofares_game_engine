#pragma once
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <vector>

namespace utils
{

glm::vec2 GetCenterOfRect(const SDL_Rect& rect);

// Split rect into m x n smaller rects.
std::vector<SDL_Rect> SplitRect(const SDL_Rect& rect, int m, int n);

// Function to divide an SDL_Rect into smaller rectangles based on cell size.
std::vector<SDL_Rect> DivideRectByCellSize(const SDL_Rect& rect, const SDL_Point& cellSize);

} // namespace utils
