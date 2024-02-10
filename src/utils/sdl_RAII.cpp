#include "sdl_RAII.h"
#include "my_common_cpp_utils/Logger.h"
#include <stdexcept>

SDLInitializer::SDLInitializer(Uint32 flags)
{
    if (SDL_Init(flags) < 0)
    {
        throw std::runtime_error(MY_FMT("SDL could not initialize! SDL_Error: {}", SDL_GetError()));
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
        throw std::runtime_error(MY_FMT("Failed to create SDL Window: {}", SDL_GetError()));
    }
}

SDLRenderer::SDLRenderer(SDL_Window* window)
{
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        throw std::runtime_error(MY_FMT("Failed to create SDL Renderer: {}", SDL_GetError()));
    }
}

SDLRenderer::~SDLRenderer()
{
    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
    }
}

Texture::Texture(SDL_Texture* texture) : texture(texture)
{
    if (!texture)
        throw std::runtime_error("Texture is nullptr");

    MY_LOG_FMT(trace, "Texture created: {}", static_cast<void*>(texture));
}

Texture::~Texture()
{
    if (texture)
    {
        SDL_DestroyTexture(texture);
    }

    MY_LOG_FMT(trace, "Texture destroyed: {}", static_cast<void*>(texture));
}

// NOTE: SDLTexture(SDLTexture&& other) noexcept : texture(other.texture) { other.texture = nullptr; }
Texture::Texture(Texture&& other) noexcept : texture(std::exchange(other.texture, nullptr))
{}

// NOTE:
// SDLTexture& operator=(SDLTexture&& other) noexcept
// {
//     if (this != &other)
//     {
//         SDL_DestroyTexture(texture);
//         texture = other.texture;
//         other.texture = nullptr;
//     }
//     return *this;
// }
Texture& Texture::operator=(Texture&& other) noexcept
{
    std::swap(texture, other.texture);
    return *this;
}
