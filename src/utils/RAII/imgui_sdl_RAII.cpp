#include "imgui_sdl_RAII.h"
#include <stdexcept>

ImGuiSDLRAII::ImGuiSDLRAII(SDL_Window* window, SDL_Renderer* renderer) : renderer(renderer)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    if (!ImGui_ImplSDL2_InitForSDLRenderer(window, renderer))
        throw std::runtime_error("Failed to initialize ImGui SDL2 backend");

    if (!ImGui_ImplSDLRenderer2_Init(renderer))
        throw std::runtime_error("Failed to initialize ImGui SDL Renderer backend");
}

ImGuiSDLRAII::~ImGuiSDLRAII()
{
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiSDLRAII::startFrame() const
{
    // Prepares a new frame for ImGui rendering with SDL_Renderer backend.
    // This function should be called before any ImGui drawing commands in the new frame.
    // It sets up necessary state for ImGui rendering with SDL_Renderer.
    ImGui_ImplSDLRenderer2_NewFrame();

    // Updates ImGui with the latest input events from SDL and starts a new ImGui frame.
    // This includes processing keyboard, mouse, and other input devices to update ImGui's internal state.
    // It should be called once per frame, before any ImGui rendering commands.
    ImGui_ImplSDL2_NewFrame();

    // Starts a new ImGui frame and prepares for new UI rendering in this frame.
    // This function must be called once per frame before any ImGui rendering commands (e.g., ImGui::Begin(),
    // ImGui::Button()). It sets up the necessary state for the ImGui frame, handles timing, and processes inputs,
    // making ImGui ready to accept your UI commands.
    ImGui::NewFrame();
}

void ImGuiSDLRAII::finishFrame() const
{
    // Renders the ImGui draw commands submitted between ImGui::NewFrame() and ImGui::Render() calls and prepares
    // the draw data for display. This function does not perform the actual drawing on the screen but instead
    // generates the draw data that will be used by the rendering backend (e.g., OpenGL, DirectX, SDL_Renderer) to
    // display the ImGui elements. It should be called after you have finished submitting all your ImGui UI elements
    // for the current frame and before the rendering backend's function to display ImGui elements on the screen.
    ImGui::Render();

    // Question: Why is this here?
    // Render the scene with double buffering
    // Renders the ImGui draw data using the SDL_Renderer backend.
    // This function draws all ImGui elements prepared in the current frame onto the screen.
    // It should be called after all ImGui rendering commands and before SDL_RenderPresent to display ImGui
    // elements.
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

    // Updates the screen with rendering performed since the last call.
    // This function presents the final image to the screen, including both your application content and the
    // ImGui overlay. It should be the last call in your rendering loop to display everything rendered in the
    // current frame.
    SDL_RenderPresent(renderer);
}
