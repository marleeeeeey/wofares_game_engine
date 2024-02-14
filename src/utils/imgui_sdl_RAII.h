#pragma once
#include <SDL.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

class ImGuiSDLRAII
{
    SDL_Renderer* renderer = nullptr;
public:
    ImGuiSDLRAII(SDL_Window* window, SDL_Renderer* renderer);
    ~ImGuiSDLRAII();

    void startFrame() const;
    void finishFrame() const;
};
