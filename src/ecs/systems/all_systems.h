#pragma once
#include <ecs/components/all_components.h>
#include <entt/entt.hpp>
#include <glm/gtc/random.hpp>
#include <imgui_impl_sdl2.h>
#include <my_common_cpp_utils/Logger.h>
#include <SDL.h>

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

        glm::vec2 renderPosition = position.value - worldScale.cameraCenter;
        glm::vec2 scaledPosition = renderPosition * worldScale.worldScale;

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
            const float scaleSpeed = 0.1f;
            if ( event.wheel.y > 0 )
                gameState.worldScale += scaleSpeed;
            else if ( event.wheel.y < 0 )
                gameState.worldScale -= scaleSpeed;
            gameState.worldScale = glm::clamp( gameState.worldScale, 0.5f, 3.0f );
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
            gameState.debugMsg =
                "Mouse moved to (" + std::to_string( event.motion.x ) + ", " + std::to_string( event.motion.y ) + ")";
            const float cameraSpeed = 0.5f;
            gameState.cameraCenter.x -= event.motion.xrel * cameraSpeed;
            gameState.cameraCenter.y -= event.motion.yrel * cameraSpeed;
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
    ImGui::Text( MY_FMT( "World Scale: {:.2f}", gameState.worldScale ).c_str() );
    ImGui::Text( MY_FMT( "Debug Message: {}", gameState.debugMsg ).c_str() );
    ImGui::Text( MY_FMT( "Camera Center: {}", gameState.cameraCenter ).c_str() );
    ImGui::Text( MY_FMT( "Scene Captured: {}", gameState.isSceneCaptured ).c_str() );

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
