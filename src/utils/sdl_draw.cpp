#include "sdl_draw.h"

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
