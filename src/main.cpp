#include <SDL.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <iostream>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float GRAVITY = 1000.0f;
const float JUMP_VELOCITY = -600.0f;
const float MOVE_SPEED = 300.0f;
const int FPS = 60;
const int FRAME_DELAY = 1000 / FPS;

struct Position
{
    glm::vec2 value;
};

struct Velocity
{
    glm::vec2 value;
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
    SDLWindow( const std::string& title, int width, int height )
    {
        window = SDL_CreateWindow(
            title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN );
        if ( !window )
        {
            throw std::runtime_error( "Failed to create SDL Window: " + std::string( SDL_GetError() ) );
        }
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

void BoundarySystem( entt::registry& registry, float screenWidth, float screenHeight )
{
    auto view = registry.view<Position>();
    for ( auto entity : view )
    {
        auto& position = view.get<Position>( entity );

        if ( position.value.x < 0 )
        {
            position.value.x = 0;
        }
        else if ( position.value.x > screenWidth )
        {
            position.value.x = screenWidth;
        }

        if ( position.value.y < 0 )
        {
            position.value.y = 0;
        }
        else if ( position.value.y > screenHeight )
        {
            position.value.y = screenHeight;
        }
    }
}

void InputSystem( entt::registry& registry )
{
    const Uint8* currentKeyStates = SDL_GetKeyboardState( NULL );

    auto view = registry.view<Velocity>();
    for ( auto entity : view )
    {
        auto& vel = view.get<Velocity>( entity );

        if ( currentKeyStates[SDL_SCANCODE_UP] )
        {
            vel.value.y = JUMP_VELOCITY;
        }
        if ( currentKeyStates[SDL_SCANCODE_LEFT] )
        {
            vel.value.x = -MOVE_SPEED;
        }
        else if ( currentKeyStates[SDL_SCANCODE_RIGHT] )
        {
            vel.value.x = MOVE_SPEED;
        }
        else
        {
            vel.value.x = 0;
        }
    }
}

void PhysicsSystem( entt::registry& registry, float deltaTime )
{
    auto view = registry.view<Position, Velocity>();
    for ( auto entity : view )
    {
        auto& pos = view.get<Position>( entity ).value;
        auto& vel = view.get<Velocity>( entity ).value;

        vel.y += GRAVITY * deltaTime;
        pos += vel * deltaTime;
    }
}

int main( int argc, char* args[] )
{
    try
    {
        SDLInitializer sdlInitializer( SDL_INIT_VIDEO );
        SDLWindow window( "Bouncing Ball with SDL, EnTT & GLM", WINDOW_WIDTH, WINDOW_HEIGHT );
        SDLRenderer renderer( window.get() );

        entt::registry registry;
        auto ball = registry.create();
        registry.emplace<Position>( ball, glm::vec2( WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 ) );
        registry.emplace<Velocity>( ball, glm::vec2( 0, 0 ) );

        Uint32 lastTick = SDL_GetTicks();
        bool quit = false;
        SDL_Event e;

        while ( !quit )
        {
            Uint32 frameStart = SDL_GetTicks();

            while ( SDL_PollEvent( &e ) != 0 )
            {
                if ( e.type == SDL_QUIT )
                {
                    quit = true;
                }
            }

            InputSystem( registry );

            Uint32 currentTick = SDL_GetTicks();
            float deltaTime = static_cast<float>( currentTick - lastTick ) / 1000.0f;
            lastTick = currentTick;

            PhysicsSystem( registry, deltaTime );

            // Fill the background with white
            SDL_SetRenderDrawColor( renderer.get(), 255, 255, 255, 255 );
            SDL_RenderClear( renderer.get() );

            BoundarySystem( registry, WINDOW_WIDTH, WINDOW_HEIGHT );
            RenderSystem( registry, renderer.get() );

            // Render the scene with double buffering
            SDL_RenderPresent( renderer.get() );

            Uint32 frameTime = SDL_GetTicks() - frameStart;
            if ( FRAME_DELAY > frameTime )
            {
                SDL_Delay( FRAME_DELAY - frameTime );
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
