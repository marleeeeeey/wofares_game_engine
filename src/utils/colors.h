#include <SDL.h>

enum class ColorName
{
    White,
    Red,
    Green,
    Blue,
    Black,
    Yellow,
    Cyan,
    Magenta,
    Orange,
    Purple,
    Grey,
    DarkGreen,
    Navy,
    Maroon,
    Olive,
    Teal,
    Silver,
    Lime,
    Aqua,
    Fuchsia,
};

SDL_Color GetSDLColor(ColorName colorName, Uint8 alpha = 255);

void SetRenderDrawColor(SDL_Renderer* renderer, const SDL_Color& color);

void SetRenderDrawColor(SDL_Renderer* renderer, ColorName colorName);
