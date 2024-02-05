#pragma once
#include <SDL.h>
#include <glm/glm.hpp>
#include <stdexcept>


class SDLInitializer
{
public:
    explicit SDLInitializer(Uint32 flags)
    {
        if (SDL_Init(flags) < 0)
        {
            throw std::runtime_error("SDL could not initialize! SDL_Error: " + std::string(SDL_GetError()));
        }
    }

    ~SDLInitializer() { SDL_Quit(); }

    SDLInitializer(const SDLInitializer&) = delete;
    SDLInitializer& operator=(const SDLInitializer&) = delete;
};

class SDLWindow
{
public:
    SDLWindow(const std::string& title, int width, int height) { init(title, width, height); }

    SDLWindow(const std::string& title, glm::vec2 windowSize)
    {
        init(title, static_cast<int>(windowSize.x), static_cast<int>(windowSize.y));
    }

    ~SDLWindow()
    {
        if (window)
        {
            SDL_DestroyWindow(window);
        }
    }

    [[nodiscard]] SDL_Window* get() const { return window; }

    SDLWindow(const SDLWindow&) = delete;
    SDLWindow& operator=(const SDLWindow&) = delete;
private:
    void init(const std::string& title, int width, int height)
    {
        window = SDL_CreateWindow(
            title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
        if (!window)
        {
            throw std::runtime_error("Failed to create SDL Window: " + std::string(SDL_GetError()));
        }
    }

    SDL_Window* window = nullptr;
};

class SDLRenderer
{
public:
    explicit SDLRenderer(SDL_Window* window)
    {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer)
        {
            throw std::runtime_error("Failed to create SDL Renderer: " + std::string(SDL_GetError()));
        }
    }

    ~SDLRenderer()
    {
        if (renderer)
        {
            SDL_DestroyRenderer(renderer);
        }
    }

    [[nodiscard]] SDL_Renderer* get() const { return renderer; }

    SDLRenderer(const SDLRenderer&) = delete;
    SDLRenderer& operator=(const SDLRenderer&) = delete;
private:
    SDL_Renderer* renderer = nullptr;
};
