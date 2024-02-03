#include <SDL.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <iostream>
#include <imgui.h>
#include <string>

#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

struct Position
{
    glm::vec2 value;
};

struct Velocity
{
    glm::vec2 value;
};

struct GameState
{
    bool quit{ false }; // Flag to control game loop exit
    glm::vec2 windowSize{ 800, 600 };
    unsigned fps{ 60 };
    float gravity = 1000.f;
    float worldScale{ 1.0f };
    std::string debugMsg;
};

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
        {
            throw std::runtime_error( "Failed to initialize ImGui SDL2 backend" );
        }
        if ( !ImGui_ImplSDLRenderer2_Init( renderer ) )
        {
            throw std::runtime_error( "Failed to initialize ImGui SDL Renderer backend" );
        }
    }

    ~ImGuiSDL()
    {
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }

    void startNewFrame() const
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

void RenderSystem( entt::registry& registry, SDL_Renderer* renderer )
{
    // Fill the background with white.
    SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );
    SDL_RenderClear( renderer );

    auto& worldScale = registry.get<GameState>( registry.view<GameState>().front() );

    auto view = registry.view<Position>();
    for ( auto entity : view )
    {
        auto& position = view.get<Position>( entity );

        glm::vec2 scaledPosition = position.value * worldScale.worldScale;

        SDL_Rect rect = {
            static_cast<int>( scaledPosition.x ) - 25, static_cast<int>( scaledPosition.y ) - 25, 50, 50 };
        SDL_SetRenderDrawColor( renderer, 255, 0, 0, 255 );
        SDL_RenderFillRect( renderer, &rect );
    }
}

void BoundarySystem( entt::registry& registry, const glm::vec2& windowSize )
{
    auto view = registry.view<Position>();
    for ( auto entity : view )
    {
        auto& position = view.get<Position>( entity );

        if ( position.value.x < 0 )
        {
            position.value.x = 0;
        }
        else if ( position.value.x > windowSize.x )
        {
            position.value.x = windowSize.x;
        }

        if ( position.value.y < 0 )
        {
            position.value.y = 0;
        }
        else if ( position.value.y > windowSize.y )
        {
            position.value.y = windowSize.y;
        }
    }
}

void InputSystem( entt::registry& registry )
{
    const float jumpVelocity = -600.0f;
    const float moveSpeed = 300.0f;

    const Uint8* currentKeyStates = SDL_GetKeyboardState( nullptr );

    auto view = registry.view<Velocity>();
    for ( auto entity : view )
    {
        auto& vel = view.get<Velocity>( entity );

        if ( currentKeyStates[SDL_SCANCODE_UP] )
        {
            vel.value.y = jumpVelocity;
        }
        if ( currentKeyStates[SDL_SCANCODE_LEFT] )
        {
            vel.value.x = -moveSpeed;
        }
        else if ( currentKeyStates[SDL_SCANCODE_RIGHT] )
        {
            vel.value.x = moveSpeed;
        }
        else
        {
            vel.value.x = 0;
        }
    }
}

void PhysicsSystem( entt::registry& registry, float deltaTime )
{
    auto gameStateEntity = registry.view<GameState>().front();
    const auto& gameState = registry.get<GameState>( gameStateEntity );

    auto view = registry.view<Position, Velocity>();
    for ( auto entity : view )
    {
        auto& pos = view.get<Position>( entity ).value;
        auto& vel = view.get<Velocity>( entity ).value;

        vel.y += gameState.gravity * deltaTime;
        pos += vel * deltaTime;
    }
}

void EventSystem( entt::registry& registry, entt::dispatcher& dispatcher )
{
    const float scaleSpeed = 0.1f;

    auto& gameState = registry.get<GameState>( registry.view<GameState>().front() );

    SDL_Event event;
    while ( SDL_PollEvent( &event ) )
    {
        ImGui_ImplSDL2_ProcessEvent( &event );
        if ( event.type == SDL_QUIT )
        {
            gameState.quit = true;
        }
        else if ( event.type == SDL_MOUSEWHEEL )
        {
            gameState.debugMsg =
                "Mouse Motion: " + std::to_string( event.motion.x ) + ", " + std::to_string( event.motion.y );

            if ( event.wheel.y > 0 )
            {
                gameState.worldScale += scaleSpeed;
            }
            else if ( event.wheel.y < 0 )
            {
                gameState.worldScale -= scaleSpeed;
            }
            gameState.worldScale = glm::clamp( gameState.worldScale, 0.5f, 3.0f );
        }
    }
}

void RenderHUDSystem( entt::registry& registry, SDL_Renderer* renderer )
{
    auto& gameState = registry.get<GameState>( registry.view<GameState>().front() );

    ImGui::Begin( "HUD" );
    ImGui::Text( "Quit: %s", gameState.quit ? "true" : "false" );
    ImGui::Text(
        "Window Size: %dx%d", static_cast<int>( gameState.windowSize.x ), static_cast<int>( gameState.windowSize.y ) );
    ImGui::Text( "FPS: %u", gameState.fps );
    ImGui::Text( "Gravity: %.2f", gameState.gravity );
    ImGui::Text( "World Scale: %.2f", gameState.worldScale );
    ImGui::Text( "Debug Message: %s", gameState.debugMsg.c_str() );

    if ( ImGui::Button( "Add Random Entity" ) )
    {
        auto newEntity = registry.create();
        glm::vec2 randomPosition = glm::linearRand( glm::vec2( 0.0f, 0.0f ), gameState.windowSize );
        registry.emplace<Position>( newEntity, randomPosition );
    }

    if ( ImGui::Button( "Remove All Entities With Only Position" ) )
    {
        auto positionEntities = registry.view<Position>();

        for ( auto entity : positionEntities )
        {
            if ( !registry.any_of<Velocity>( entity ) )
            {
                registry.remove<Position>( entity );
            }
        }
    }

    ImGui::End();
}

void ScatterSystem( entt::registry& registry, const glm::vec2& windowSize )
{
    auto view = registry.view<Position>();
    for ( auto entity : view )
    {
        auto& pos = view.get<Position>( entity );

        pos.value.x = glm::linearRand( 0.0f, windowSize.x );
        pos.value.y = glm::linearRand( 0.0f, windowSize.y );
    }
}

int main( int argc, char* args[] )
{
    try
    {
        entt::registry registry;
        entt::dispatcher dispatcher;

        // Create a game state entity.
        auto& gameState = registry.emplace<GameState>( registry.create() );

        // Initialize SDL, create a window and a renderer. Initialize ImGui.
        SDLInitializer sdlInitializer( SDL_INIT_VIDEO );
        SDLWindow window( "Bouncing Ball with SDL, ImGui, EnTT & GLM", gameState.windowSize );
        SDLRenderer renderer( window.get() );
        ImGuiSDL imguiSDL( window.get(), renderer.get() );

        // Create a ball entity with position and velocity components.
        auto ball = registry.create();
        registry.emplace<Position>( ball, gameState.windowSize / 2.0f );
        registry.emplace<Velocity>( ball, glm::vec2( 0, 0 ) );

        // Create 10 entities with position components and scatter them randomly.
        for ( int i = 0; i < 10; ++i )
            registry.emplace<Position>( registry.create() );
        ScatterSystem( registry, gameState.windowSize );

        Uint32 lastTick = SDL_GetTicks();

        // Start the game loop.
        while ( !gameState.quit )
        {
            Uint32 frameStart = SDL_GetTicks();

            EventSystem( registry, dispatcher );
            InputSystem( registry );

            // Calculate delta time.
            Uint32 currentTick = SDL_GetTicks();
            float deltaTime = static_cast<float>( currentTick - lastTick ) / 1000.0f;
            lastTick = currentTick;

            // Update the physics.
            PhysicsSystem( registry, deltaTime );
            BoundarySystem( registry, gameState.windowSize );

            // Render the scene and the HUD.
            imguiSDL.startNewFrame();
            RenderSystem( registry, renderer.get() );
            RenderHUDSystem( registry, renderer.get() );
            imguiSDL.finishFrame();

            // Cap the frame rate.
            Uint32 frameTime = SDL_GetTicks() - frameStart;
            const Uint32 frameDelay = 1000 / gameState.fps;
            if ( frameDelay > frameTime )
            {
                SDL_Delay( frameDelay - frameTime );
            }
        }
    }
    catch ( const std::runtime_error& e )
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}
