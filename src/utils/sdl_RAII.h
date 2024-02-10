#pragma once
#include "SDL_render.h"
#include <SDL.h>
#include <glm/glm.hpp>
#include <string>

class SDLInitializer
{
public:
    explicit SDLInitializer(Uint32 flags);
    ~SDLInitializer();
    SDLInitializer(const SDLInitializer&) = delete;
    SDLInitializer& operator=(const SDLInitializer&) = delete;
};

class SDLWindow
{
public:
    SDLWindow(const std::string& title, int width, int height);
    SDLWindow(const std::string& title, glm::vec2 windowSize);
    ~SDLWindow();
    SDLWindow(const SDLWindow&) = delete;
    SDLWindow& operator=(const SDLWindow&) = delete;

    [[nodiscard]] SDL_Window* get() const { return window; }
private:
    void init(const std::string& title, int width, int height);

    SDL_Window* window = nullptr;
};

class SDLRenderer
{
public:
    explicit SDLRenderer(SDL_Window* window);
    ~SDLRenderer();
    SDLRenderer(const SDLRenderer&) = delete;
    SDLRenderer& operator=(const SDLRenderer&) = delete;

    [[nodiscard]] SDL_Renderer* get() const { return renderer; }
private:
    SDL_Renderer* renderer = nullptr;
};

class Texture
{
public:
    Texture(SDL_Texture* texture);
    ~Texture();
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    [[nodiscard]] SDL_Texture* get() const { return texture; }
private:
    SDL_Texture* texture = nullptr;
};
