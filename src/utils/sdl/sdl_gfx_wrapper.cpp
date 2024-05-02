#include "sdl_gfx_wrapper.h"
#include <SDL2_gfxPrimitives.h>

int DrawPoligon(SDL_Renderer* renderer, std::vector<glm::vec2> vertices, SDL_Color color, FillOption fillOption)
{
    int n = vertices.size();

    std::vector<int16_t> vertX(n);
    std::vector<int16_t> vertY(n);
    for (int i = 0; i < n; i++)
    {
        vertX[i] = vertices[i].x;
        vertY[i] = vertices[i].y;
    }

    if (fillOption == FillOption::Filled)
        return filledPolygonRGBA(renderer, vertX.data(), vertY.data(), n, color.r, color.g, color.b, color.a);
    else
        return polygonRGBA(renderer, vertX.data(), vertY.data(), n, color.r, color.g, color.b, color.a);
}

int DrawCircle(SDL_Renderer* renderer, glm::vec2 center, float radius, SDL_Color color, FillOption fillOption)
{
    if (fillOption == FillOption::Filled)
        return filledCircleRGBA(renderer, center.x, center.y, radius, color.r, color.g, color.b, color.a);
    else
        return circleRGBA(renderer, center.x, center.y, radius, color.r, color.g, color.b, color.a);
}

void DrawCross(SDL_Renderer* renderer, int centerX, int centerY, int size, const SDL_Color& color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLine(renderer, centerX - size / 2, centerY, centerX + size / 2, centerY);
    SDL_RenderDrawLine(renderer, centerX, centerY - size / 2, centerX, centerY + size / 2);
}

void DrawCross(SDL_Renderer* renderer, const glm::vec2& center, int size, const SDL_Color& color)
{
    DrawCross(renderer, static_cast<int>(center.x), static_cast<int>(center.y), size, color);
}
