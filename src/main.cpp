#include <cstddef>
#include <ecs/components/all_components.h>
#include <ecs/systems/all_systems.h>
#include <my_common_cpp_utils/Logger.h>
#include <utils/imgui_sdl_RAII.h>
#include <utils/sdl_RAII.h>

int main(int argc, char* args[])
{
    try
    {
        entt::registry registry;
        entt::dispatcher dispatcher;

        // Create a game state entity.
        auto& gameState = registry.emplace<GameState>(registry.create());
        gameState.cameraCenter = gameState.windowSize / 2.0f;

        // Initialize SDL, create a window and a renderer. Initialize ImGui.
        SDLInitializer sdlInitializer(SDL_INIT_VIDEO);
        SDLWindow window("Bouncing Ball with SDL, ImGui, EnTT & GLM", gameState.windowSize);
        SDLRenderer renderer(window.get());
        ImGuiSDL imguiSDL(window.get(), renderer.get());

        // Create a ball entity with position and velocity components.
        auto ball = registry.create();
        registry.emplace<Position>(ball, gameState.windowSize / 2.0f);
        registry.emplace<Velocity>(ball, glm::vec2(0, 0));
        registry.emplace<SizeComponent>(ball, glm::vec2(32, 32));
        registry.emplace<PlayerNumber>(ball, size_t{1});

        LoadMap(registry, "C:\\dev\\my_tiled_maps\\map002_wofares\\map.json");

        Uint32 lastTick = SDL_GetTicks();

        // Start the game loop.
        while (!gameState.quit)
        {
            Uint32 frameStart = SDL_GetTicks();

            EventSystem(registry, dispatcher);
            InputSystem(registry);

            // Calculate delta time.
            Uint32 currentTick = SDL_GetTicks();
            float deltaTime = static_cast<float>(currentTick - lastTick) / 1000.0f;
            lastTick = currentTick;

            // Update the physics.
            PhysicsSystem(registry, deltaTime);
            BoundarySystem(registry, gameState.windowSize);
            CollisionSystem(registry);

            // Render the scene and the HUD.
            imguiSDL.startFrame();
            RenderSystem(registry, renderer.get());
            DrawGridSystem(renderer.get(), gameState);
            RenderHUDSystem(registry, renderer.get());
            imguiSDL.finishFrame();

            // Cap the frame rate.
            Uint32 frameTime = SDL_GetTicks() - frameStart;
            const Uint32 frameDelay = 1000 / gameState.fps;
            if (frameDelay > frameTime)
            {
                SDL_Delay(frameDelay - frameTime);
            }
        }
    }
    catch (const std::runtime_error& e)
    {
        MY_LOG_FMT(warn, "[main] Unhandled exception: {}", e.what());
        return -1;
    }

    return 0;
}
