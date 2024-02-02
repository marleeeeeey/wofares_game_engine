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
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Bouncing Ball with SDL, EnTT & GLM", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH,
        WINDOW_HEIGHT, SDL_WINDOW_SHOWN );
    SDL_Renderer* renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );

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

        SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );
        SDL_RenderClear( renderer );

        BoundarySystem( registry, WINDOW_WIDTH, WINDOW_HEIGHT );
        RenderSystem( registry, renderer );

        SDL_RenderPresent( renderer );

        SDL_Delay( 16 ); // ~60 frames per second
    }

    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_Quit();

    return 0;
}
