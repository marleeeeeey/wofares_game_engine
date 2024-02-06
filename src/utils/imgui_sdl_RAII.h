#pragma once
#include <SDL.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

class ImGuiSDL
{
    SDL_Renderer* renderer = nullptr;
public:
    ImGuiSDL(SDL_Window* window, SDL_Renderer* renderer);
    ~ImGuiSDL();

    void startFrame() const;
    void finishFrame() const;
};
