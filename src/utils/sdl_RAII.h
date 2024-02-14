#pragma once
#include "SDL_render.h"
#include <SDL.h>
#include <glm/glm.hpp>
#include <string>

class SDLInitializerRAII
{
public:
    explicit SDLInitializerRAII(Uint32 flags);
    ~SDLInitializerRAII();
    SDLInitializerRAII(const SDLInitializerRAII&) = delete;
    SDLInitializerRAII& operator=(const SDLInitializerRAII&) = delete;
};

class SDLWindowRAII
{
public:
    SDLWindowRAII(const std::string& title, int width, int height);
    SDLWindowRAII(const std::string& title, glm::vec2 windowSize);
    ~SDLWindowRAII();
    SDLWindowRAII(const SDLWindowRAII&) = delete;
    SDLWindowRAII& operator=(const SDLWindowRAII&) = delete;

    [[nodiscard]] SDL_Window* get() const { return window; }
private:
    void init(const std::string& title, int width, int height);

    SDL_Window* window = nullptr;
};

class SDLRendererRAII
{
public:
    explicit SDLRendererRAII(SDL_Window* window);
    ~SDLRendererRAII();
    SDLRendererRAII(const SDLRendererRAII&) = delete;
    SDLRendererRAII& operator=(const SDLRendererRAII&) = delete;

    [[nodiscard]] SDL_Renderer* get() const { return renderer; }
private:
    SDL_Renderer* renderer = nullptr;
};

class SDLTextureRAII
{
public:
    SDLTextureRAII(SDL_Texture* texture);
    ~SDLTextureRAII();
    SDLTextureRAII(SDLTextureRAII&& other) noexcept;
    SDLTextureRAII& operator=(SDLTextureRAII&& other) noexcept;

    [[nodiscard]] SDL_Texture* get() const { return texture; }
private:
    SDL_Texture* texture = nullptr;
};

class SDLTextureLockRAII
{
public:
    SDLTextureLockRAII(SDL_Texture* texture);
    ~SDLTextureLockRAII();
    SDLTextureLockRAII(const SDLTextureLockRAII&) = delete;
    SDLTextureLockRAII& operator=(const SDLTextureLockRAII&) = delete;

    void* GetPixels() const { return pixels; }
    int GetPitch() const { return pitch; }
private:
    SDL_Texture* texture;
    void* pixels;
    int pitch; // Number of bytes in a row of pixel data, including padding between lines.
};
