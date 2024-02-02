#include <SDL.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <iostream>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float GRAVITY = 0.5f;
const float JUMP_VELOCITY = -15.0f;
const float MOVE_SPEED = 5.0f;

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

        bool quit = false;
        SDL_Event e;

        while ( !quit )
        {
            while ( SDL_PollEvent( &e ) != 0 )
            {
                if ( e.type == SDL_QUIT )
                {
                    quit = true;
                }
            }

            const Uint8* currentKeyStates = SDL_GetKeyboardState( NULL );
            if ( currentKeyStates[SDL_SCANCODE_UP] )
            {
                registry.get<Velocity>( ball ).value.y = JUMP_VELOCITY;
            }
            if ( currentKeyStates[SDL_SCANCODE_LEFT] )
            {
                registry.get<Velocity>( ball ).value.x = -MOVE_SPEED;
            }
            else if ( currentKeyStates[SDL_SCANCODE_RIGHT] )
            {
                registry.get<Velocity>( ball ).value.x = MOVE_SPEED;
            }
            else
            {
                registry.get<Velocity>( ball ).value.x = 0;
            }

            // Update physics
            auto& pos = registry.get<Position>( ball );
            auto& vel = registry.get<Velocity>( ball );

            vel.value.y += GRAVITY; // Apply gravity
            pos.value += vel.value; // Update position

            SDL_SetRenderDrawColor( renderer.get(), 255, 255, 255, 255 );
            SDL_RenderClear( renderer.get() );

            BoundarySystem( registry, WINDOW_WIDTH, WINDOW_HEIGHT );
            RenderSystem( registry, renderer.get() );

            SDL_RenderPresent( renderer.get() );

            SDL_Delay( 16 ); // ~60 frames per second
        }
    }
    catch ( const std::runtime_error& e )
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}
