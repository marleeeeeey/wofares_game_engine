#include <SDL.h>
#include <glm/glm.hpp>
#include <iostream>

int main( int argc, char* argv[] )
{
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Bouncing Ball", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN );
    if ( !window )
    {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
    if ( !renderer )
    {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    bool quit = false;
    SDL_Event e;
    glm::vec2 ballPos( 400, 300 );
    glm::vec2 ballVel( 0, -5 );

    while ( !quit )
    {
        while ( SDL_PollEvent( &e ) != 0 )
        {
            if ( e.type == SDL_QUIT )
            {
                quit = true;
            }
        }

        ballPos += ballVel;
        ballVel.y += 0.2f; // Gravity effect

        // Bounce off the floor and ceiling
        if ( ballPos.y >= 600 || ballPos.y <= 0 )
        {
            ballVel.y = -ballVel.y;
        }

        SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );
        SDL_RenderClear( renderer );

        // Draw the ball
        SDL_Rect ballRect = { static_cast<int>( ballPos.x - 10 ), static_cast<int>( ballPos.y - 10 ), 20, 20 };
        SDL_SetRenderDrawColor( renderer, 255, 0, 0, 255 );
        SDL_RenderFillRect( renderer, &ballRect );

        SDL_RenderPresent( renderer );

        SDL_Delay( 16 ); // ~60 frames per second
    }

    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_Quit();

    return 0;
}
