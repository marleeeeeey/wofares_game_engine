#include "sdl_RAII.h"
#include <stdexcept>

SDLInitializer::SDLInitializer(Uint32 flags)
{
    if (SDL_Init(flags) < 0)
    {
        throw std::runtime_error("SDL could not initialize! SDL_Error: " + std::string(SDL_GetError()));
    }
}

SDLInitializer::~SDLInitializer()
{
    SDL_Quit();
}

SDLWindow::SDLWindow(const std::string& title, int width, int height)
{
    init(title, width, height);
}

SDLWindow::SDLWindow(const std::string& title, glm::vec2 windowSize)
{
    init(title, static_cast<int>(windowSize.x), static_cast<int>(windowSize.y));
}

SDLWindow::~SDLWindow()
{
    if (window)
    {
        SDL_DestroyWindow(window);
    }
}

void SDLWindow::init(const std::string& title, int width, int height)
{
    window = SDL_CreateWindow(
        title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (!window)
    {
        throw std::runtime_error("Failed to create SDL Window: " + std::string(SDL_GetError()));
    }
}

SDLRenderer::SDLRenderer(SDL_Window* window)
{
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        throw std::runtime_error("Failed to create SDL Renderer: " + std::string(SDL_GetError()));
    }
}
SDLRenderer::~SDLRenderer()
{
    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
    }
}
