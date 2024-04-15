#pragma once
#include <SDL.h>
#include <SDL_pixels.h>
#include <SDL_render.h>
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
    SDL_Window* window = nullptr;
public:
    SDLWindowRAII(const std::string& title, int width, int height);
    SDLWindowRAII(const std::string& title, glm::vec2 windowSize);
    ~SDLWindowRAII();
    SDLWindowRAII(const SDLWindowRAII&) = delete;
    SDLWindowRAII& operator=(const SDLWindowRAII&) = delete;
public:
    [[nodiscard]] SDL_Window* get() const { return window; }
    operator SDL_Window*() const { return window; } // TODO3: decide if the same way can be used for other RAII classes.
private:
    void init(const std::string& title, int width, int height);
};

class SDLRendererRAII
{
    SDL_Renderer* renderer = nullptr;
public:
    explicit SDLRendererRAII(SDL_Window* window);
    ~SDLRendererRAII();
    SDLRendererRAII(const SDLRendererRAII&) = delete;
    SDLRendererRAII& operator=(const SDLRendererRAII&) = delete;
public:
    [[nodiscard]] SDL_Renderer* get() const { return renderer; }
};

class SDLTextureRAII
{
    SDL_Texture* texture = nullptr;
public:
    SDLTextureRAII(SDL_Texture* texture);
    ~SDLTextureRAII();
    SDLTextureRAII(const SDLTextureRAII&) = delete;
    SDLTextureRAII& operator=(const SDLTextureRAII&) = delete;
    SDLTextureRAII(SDLTextureRAII&& other) noexcept;
    SDLTextureRAII& operator=(SDLTextureRAII&& other) noexcept;
public:
    [[nodiscard]] SDL_Texture* get() const { return texture; }
};

class SDLTextureLockRAII
{
    SDL_Texture* texture;
    void* pixels;
    int pitch; // Number of bytes in a row of pixel data, including padding between lines.
public:
    SDLTextureLockRAII(SDL_Texture* texture);
    ~SDLTextureLockRAII();
    SDLTextureLockRAII(const SDLTextureLockRAII&) = delete;
    SDLTextureLockRAII& operator=(const SDLTextureLockRAII&) = delete;
public:
    void* GetPixels() const { return pixels; }
    int GetPitch() const { return pitch; }
};

class SDLSurfaceLockRAII
{
public:
    explicit SDLSurfaceLockRAII(SDL_Surface* surface);
    ~SDLSurfaceLockRAII();
    SDLSurfaceLockRAII(const SDLSurfaceLockRAII&) = delete;
    SDLSurfaceLockRAII& operator=(const SDLSurfaceLockRAII&) = delete;
private:
    SDL_Surface* surface;
};

class SDLSurfaceRAII
{
public:
    SDLSurfaceRAII(SDL_Surface* surface);
    ~SDLSurfaceRAII();
    SDLSurfaceRAII(SDLSurfaceRAII&& other) noexcept;
    SDLSurfaceRAII& operator=(SDLSurfaceRAII&& other) noexcept;

    SDL_Surface* get() const { return surface; }
private:
    SDL_Surface* surface = nullptr;
};

class SDLPixelFormatRAII
{
public:
    SDLPixelFormatRAII(SDL_PixelFormat* format);
    ~SDLPixelFormatRAII();
    SDLPixelFormatRAII(SDLPixelFormatRAII&& other) noexcept;
    SDLPixelFormatRAII& operator=(SDLPixelFormatRAII&& other) noexcept;

    SDL_PixelFormat* get() const { return pixelFormat; }
private:
    SDL_PixelFormat* pixelFormat = nullptr;
};
