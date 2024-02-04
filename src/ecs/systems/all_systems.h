#pragma once
#include "SDL_rect.h"
#include <SDL.h>
#include <ecs/components/all_components.h>
#include <entt/entt.hpp>
#include <glm/gtc/random.hpp>
#include <imgui_impl_sdl2.h>
#include <map>
#include <my_common_cpp_utils/Logger.h>

enum class ColorName
{
    White,
    Red,
    Green,
    Blue,
    Black,
    Yellow,
    Cyan,
    Magenta,
    Orange,
    Purple,
    Grey,
    DarkGreen,
    Navy,
    Maroon,
    Olive,
    Teal,
    Silver,
    Lime,
    Aqua,
    Fuchsia,
};

SDL_Color GetSDLColor( ColorName colorName, Uint8 alpha = 255 )
{
    static const std::map<ColorName, SDL_Color> colorMap = {
        { ColorName::White, { 255, 255, 255, 255 } },  { ColorName::Red, { 255, 0, 0, 255 } },
        { ColorName::Green, { 0, 255, 0, 255 } },      { ColorName::Blue, { 0, 0, 255, 255 } },
        { ColorName::Black, { 0, 0, 0, 255 } },        { ColorName::Yellow, { 255, 255, 0, 255 } },
        { ColorName::Cyan, { 0, 255, 255, 255 } },     { ColorName::Magenta, { 255, 0, 255, 255 } },
        { ColorName::Orange, { 255, 165, 0, 255 } },   { ColorName::Purple, { 128, 0, 128, 255 } },
        { ColorName::Grey, { 128, 128, 128, 255 } },   { ColorName::DarkGreen, { 0, 100, 0, 255 } },
        { ColorName::Navy, { 0, 0, 128, 255 } },       { ColorName::Maroon, { 128, 0, 0, 255 } },
        { ColorName::Olive, { 128, 128, 0, 255 } },    { ColorName::Teal, { 0, 128, 128, 255 } },
        { ColorName::Silver, { 192, 192, 192, 255 } }, { ColorName::Lime, { 0, 255, 0, 255 } },
        { ColorName::Aqua, { 0, 255, 255, 255 } },     { ColorName::Fuchsia, { 255, 0, 255, 255 } },
    };

    auto color = colorMap.at( colorName );
    color.a = alpha;

    return color;
}

void SetRenderDrawColor( SDL_Renderer* renderer, const SDL_Color& color )
{
    SDL_SetRenderDrawColor( renderer, color.r, color.g, color.b, color.a );
}

void SetRenderDrawColor( SDL_Renderer* renderer, ColorName colorName )
{
    SetRenderDrawColor( renderer, GetSDLColor( colorName ) );
}

void RenderSystem( entt::registry& registry, SDL_Renderer* renderer )
{
    SetRenderDrawColor( renderer, ColorName::White );
    SDL_RenderClear( renderer );

    auto& gameState = registry.get<GameState>( registry.view<GameState>().front() );

    auto view = registry.view<Position, SizeComponent>();
    for ( auto entity : view )
    {
        auto& position = view.get<Position>( entity );
        auto& size = view.get<SizeComponent>( entity );

        glm::vec2 transformedPosition =
            ( position.value - gameState.cameraCenter ) * gameState.cameraScale + gameState.windowSize / 2.0f;
        SDL_Rect rect = {
            static_cast<int>( transformedPosition.x ), static_cast<int>( transformedPosition.y ),
            static_cast<int>( size.value.x * gameState.cameraScale ),
            static_cast<int>( size.value.y * gameState.cameraScale ) };

        SetRenderDrawColor( renderer, ColorName::Red );
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
            float prevScale = gameState.cameraScale;

            // Calculate the new scale of the camera
            const float scaleSpeed = 0.1f;
            if ( event.wheel.y > 0 )
                gameState.cameraScale += scaleSpeed;
            else if ( event.wheel.y < 0 )
                gameState.cameraScale -= scaleSpeed;
            gameState.cameraScale = glm::clamp( gameState.cameraScale, 0.5f, 3.0f );

            // Get the cursor coordinates in world coordinates
            int mouseX, mouseY;
            SDL_GetMouseState( &mouseX, &mouseY );

            glm::vec2 mouseWorldBeforeZoom =
                ( glm::vec2( mouseX, mouseY ) - gameState.windowSize * 0.5f ) / prevScale + gameState.cameraCenter;

            gameState.debugMsg2 = MY_FMT( "mouseWorldBeforeZoom {}", mouseWorldBeforeZoom );

            // Calculate the new position of the camera so that the point under the cursor remains in the same place
            gameState.cameraCenter = mouseWorldBeforeZoom -
                ( glm::vec2( mouseX, mouseY ) - gameState.windowSize * 0.5f ) / gameState.cameraScale;
        }
        else if ( event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT )
        {
            gameState.isSceneCaptured = true;
        }
        else if ( event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_RIGHT )
        {
            gameState.isSceneCaptured = false;
        }
        else if ( event.type == SDL_MOUSEMOTION && gameState.isSceneCaptured )
        {
            gameState.debugMsg = MY_FMT( "Mouse moved to ({}, {})", event.motion.x, event.motion.y );

            float deltaX = event.motion.xrel / gameState.cameraScale;
            float deltaY = event.motion.yrel / gameState.cameraScale;
            gameState.cameraCenter.x -= deltaX;
            gameState.cameraCenter.y -= deltaY;
        }
    }
}

void RenderHUDSystem( entt::registry& registry, SDL_Renderer* renderer )
{
    auto& gameState = registry.get<GameState>( registry.view<GameState>().front() );

    ImGui::Begin( "HUD" );

    ImGui::Text( MY_FMT( "Quit: {}", gameState.quit ).c_str() );
    ImGui::Text( MY_FMT( "Window Size: {}", gameState.windowSize ).c_str() );
    ImGui::Text( MY_FMT( "FPS: {}", gameState.fps ).c_str() );
    ImGui::Text( MY_FMT( "Gravity: {:.2f}", gameState.gravity ).c_str() );
    ImGui::Text( MY_FMT( "World Scale: {:.2f}", gameState.cameraScale ).c_str() );
    ImGui::Text( MY_FMT( "Camera Center: {}", gameState.cameraCenter ).c_str() );
    ImGui::Text( MY_FMT( "Scene Captured: {}", gameState.isSceneCaptured ).c_str() );
    ImGui::Text( MY_FMT( "Debug Message: {}", gameState.debugMsg ).c_str() );
    ImGui::Text( MY_FMT( "Debug Message 2: {}", gameState.debugMsg2 ).c_str() );

    // caclulare count of entities with Position:
    auto positionEntities = registry.view<Position>();
    ImGui::Text( MY_FMT( "Position Entities: {}", positionEntities.size() ).c_str() );

    if ( ImGui::Button( "Add Random Entity" ) )
    {
        auto newEntity = registry.create();
        glm::vec2 randomPosition = glm::linearRand( glm::vec2( 0.0f, 0.0f ), gameState.windowSize );
        registry.emplace<Position>( newEntity, randomPosition );
        registry.emplace<SizeComponent>( newEntity, glm::vec2( 50, 50 ) );
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

void DrawCross( SDL_Renderer* renderer, int centerX, int centerY, int size, const SDL_Color& color )
{
    SDL_SetRenderDrawColor( renderer, color.r, color.g, color.b, color.a );
    SDL_RenderDrawLine( renderer, centerX - size / 2, centerY, centerX + size / 2, centerY );
    SDL_RenderDrawLine( renderer, centerX, centerY - size / 2, centerX, centerY + size / 2 );
}

void DrawCross( SDL_Renderer* renderer, const glm::vec2& center, int size, const SDL_Color& color )
{
    DrawCross( renderer, static_cast<int>( center.x ), static_cast<int>( center.y ), size, color );
}

void DrawGridSystem( SDL_Renderer* renderer, const GameState& gameState )
{
    const int gridSize = 50;
    const SDL_Color gridColor = GetSDLColor( ColorName::Grey );
    const SDL_Color screenCenterColor = GetSDLColor( ColorName::Red );
    const SDL_Color originColor = GetSDLColor( ColorName::Green );

    // Get the window size to determine the drawing area
    int windowWidth = static_cast<int>( gameState.windowSize.x );
    int windowHeight = static_cast<int>( gameState.windowSize.y );

    auto& cameraCenter = gameState.cameraCenter;

    // Calculate the start and end points for drawing the grid
    int startX = static_cast<int>( cameraCenter.x - windowWidth / 2 / gameState.cameraScale );
    int startY = static_cast<int>( cameraCenter.y - windowHeight / 2 / gameState.cameraScale );
    int endX = startX + windowWidth / gameState.cameraScale;
    int endY = startY + windowHeight / gameState.cameraScale;

    // Align the beginning of the grid with the cell boundaries
    startX -= startX % gridSize;
    startY -= startY % gridSize;

    // Draw vertical grid lines
    SetRenderDrawColor( renderer, gridColor );
    for ( int x = startX; x <= endX; x += gridSize )
    {
        int screenX = static_cast<int>( ( x - cameraCenter.x ) * gameState.cameraScale + windowWidth / 2 );
        SDL_RenderDrawLine( renderer, screenX, 0, screenX, windowHeight );
    }

    // Draw horizontal grid lines
    for ( int y = startY; y <= endY; y += gridSize )
    {
        int screenY = static_cast<int>( ( y - cameraCenter.y ) * gameState.cameraScale + windowHeight / 2 );
        SDL_RenderDrawLine( renderer, 0, screenY, windowWidth, screenY );
    }

    // Draw the center of screen point
    DrawCross( renderer, gameState.windowSize / 2.0f, 20, screenCenterColor );

    // Draw the origin point
    glm::vec2 originWorldPos = ( -cameraCenter ) * gameState.cameraScale;
    DrawCross( renderer, originWorldPos, 20, originColor );
}
