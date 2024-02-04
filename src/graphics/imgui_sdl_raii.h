#pragma once
#include <SDL.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <stdexcept>

class SDLInitializer
{
public:
    explicit SDLInitializer( Uint32 flags )
    {
        if ( SDL_Init( flags ) < 0 )
        {
            throw std::runtime_error( "SDL could not initialize! SDL_Error: " + std::string( SDL_GetError() ) );
        }
    }

    ~SDLInitializer() { SDL_Quit(); }

    SDLInitializer( const SDLInitializer& ) = delete;
    SDLInitializer& operator=( const SDLInitializer& ) = delete;
};

class SDLWindow
{
public:
    SDLWindow( const std::string& title, int width, int height ) { init( title, width, height ); }

    SDLWindow( const std::string& title, glm::vec2 windowSize )
    {
        init( title, static_cast<int>( windowSize.x ), static_cast<int>( windowSize.y ) );
    }

    ~SDLWindow()
    {
        if ( window )
        {
            SDL_DestroyWindow( window );
        }
    }

    [[nodiscard]] SDL_Window* get() const { return window; }

    SDLWindow( const SDLWindow& ) = delete;
    SDLWindow& operator=( const SDLWindow& ) = delete;
private:
    void init( const std::string& title, int width, int height )
    {
        window = SDL_CreateWindow(
            title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN );
        if ( !window )
        {
            throw std::runtime_error( "Failed to create SDL Window: " + std::string( SDL_GetError() ) );
        }
    }

    SDL_Window* window = nullptr;
};

class SDLRenderer
{
public:
    explicit SDLRenderer( SDL_Window* window )
    {
        renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
        if ( !renderer )
        {
            throw std::runtime_error( "Failed to create SDL Renderer: " + std::string( SDL_GetError() ) );
        }
    }

    ~SDLRenderer()
    {
        if ( renderer )
        {
            SDL_DestroyRenderer( renderer );
        }
    }

    [[nodiscard]] SDL_Renderer* get() const { return renderer; }

    SDLRenderer( const SDLRenderer& ) = delete;
    SDLRenderer& operator=( const SDLRenderer& ) = delete;
private:
    SDL_Renderer* renderer = nullptr;
};

class ImGuiSDL
{
    SDL_Renderer* renderer = nullptr;
public:
    ImGuiSDL( SDL_Window* window, SDL_Renderer* renderer ) : renderer( renderer )
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        if ( !ImGui_ImplSDL2_InitForSDLRenderer( window, renderer ) )
            throw std::runtime_error( "Failed to initialize ImGui SDL2 backend" );

        if ( !ImGui_ImplSDLRenderer2_Init( renderer ) )
            throw std::runtime_error( "Failed to initialize ImGui SDL Renderer backend" );
    }

    ~ImGuiSDL()
    {
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }

    void startFrame() const
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

    void finishFrame() const
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
        ImGui_ImplSDLRenderer2_RenderDrawData( ImGui::GetDrawData() );

        // Updates the screen with rendering performed since the last call.
        // This function presents the final image to the screen, including both your application content and the
        // ImGui overlay. It should be the last call in your rendering loop to display everything rendered in the
        // current frame.
        SDL_RenderPresent( renderer );
    }
};
