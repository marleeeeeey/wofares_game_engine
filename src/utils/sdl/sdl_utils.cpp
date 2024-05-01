#include "sdl_utils.h"

namespace utils
{

glm::vec2 GetCenterOfRect(const SDL_Rect& rect)
{
    return {rect.x + rect.w / 2, rect.y + rect.h / 2};
}

std::vector<SDL_Rect> SplitRect(const SDL_Rect& rect, int m, int n)
{
    std::vector<SDL_Rect> result;
    int width = rect.w / m;
    int height = rect.h / n;

    for (int y = 0; y < n; ++y)
    {
        for (int x = 0; x < m; ++x)
        {
            SDL_Rect small_rect = {rect.x + x * width, rect.y + y * height, width, height};
            result.push_back(small_rect);
        }
    }

    return result;
}

std::vector<SDL_Rect> DivideRectByCellSize(const SDL_Rect& rect, const SDL_Point& cellSize)
{
    std::vector<SDL_Rect> cells;

    // Calculate the number of cells horizontally and vertically
    int horizontalCells = rect.w / cellSize.x;
    int verticalCells = rect.h / cellSize.y;

    // Loop through each cell and create a rectangle for it
    for (int y = 0; y < verticalCells; ++y)
    {
        for (int x = 0; x < horizontalCells; ++x)
        {
            SDL_Rect cellRect = {rect.x + x * cellSize.x, rect.y + y * cellSize.y, cellSize.x, cellSize.y};
            cells.push_back(cellRect);
        }
    }

    return cells;
}

void RotatePoint(glm::vec2& point, const glm::vec2& center, float angleRadians)
{
    float s = sin(angleRadians);
    float c = cos(angleRadians);

    // Translate point back to origin:
    point.x -= center.x;
    point.y -= center.y;

    // Rotate point
    float xnew = point.x * c - point.y * s;
    float ynew = point.x * s + point.y * c;

    // Translate point back:
    point.x = xnew + center.x;
    point.y = ynew + center.y;
}
} // namespace utils