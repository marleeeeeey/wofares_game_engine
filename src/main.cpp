#include <SDL.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <iostream>

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

void RenderSystem( entt::registry& registry, SDL_Renderer* renderer )
{
    auto view = registry.view<Position>();
    for ( auto entity : view )
    {
        auto& position = view.get<Position>( entity );

        SDL_Rect rect = {
            static_cast<int>( position.value.x ) - 25, static_cast<int>( position.value.y ) - 25, 50, 50 };
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

struct QuitEvent
{};

struct QuitEventHandler
{
    GameState& gameState;

    void handle( const QuitEvent& ) const { gameState.quit = true; }
};

void EventSystem( entt::registry& registry, entt::dispatcher& dispatcher )
{
    SDL_Event sdlEvent;
    while ( SDL_PollEvent( &sdlEvent ) != 0 )
    {
        if ( sdlEvent.type == SDL_QUIT )
        {
            dispatcher.trigger<QuitEvent>();
        }
    }
}

int main( int argc, char* args[] )
{
    try
    {
        entt::registry registry;
        entt::dispatcher dispatcher;

        // Create a game state entity and connect the quit event handler.
        auto& gameState = registry.emplace<GameState>( registry.create() );
        QuitEventHandler quitEventHandler{ gameState };
        dispatcher.sink<QuitEvent>().connect<&QuitEventHandler::handle>( quitEventHandler );

        // Initialize SDL, create a window and a renderer.
        SDLInitializer sdlInitializer( SDL_INIT_VIDEO );
        SDLWindow window( "Bouncing Ball with SDL, EnTT & GLM", gameState.windowSize );
        SDLRenderer renderer( window.get() );

        // Create a ball entity with position and velocity components.
        auto ball = registry.create();
        registry.emplace<Position>( ball, gameState.windowSize / 2.0f );
        registry.emplace<Velocity>( ball, glm::vec2( 0, 0 ) );

        Uint32 lastTick = SDL_GetTicks();

        // Start the game loop.
        while ( !gameState.quit )
        {
            Uint32 frameStart = SDL_GetTicks();

            EventSystem( registry, dispatcher );
            InputSystem( registry );

            // Calculate delta time for physics and apply physics.
            Uint32 currentTick = SDL_GetTicks();
            float deltaTime = static_cast<float>( currentTick - lastTick ) / 1000.0f;
            lastTick = currentTick;
            PhysicsSystem( registry, deltaTime );

            // Fill the background with white.
            SDL_SetRenderDrawColor( renderer.get(), 255, 255, 255, 255 );
            SDL_RenderClear( renderer.get() );

            BoundarySystem( registry, gameState.windowSize );
            RenderSystem( registry, renderer.get() );

            // Render the scene with double buffering
            SDL_RenderPresent( renderer.get() );

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
