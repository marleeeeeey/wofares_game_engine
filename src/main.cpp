#include <ecs/components/all_components.h>
#include <ecs/systems/camera_control_system.h>
#include <ecs/systems/event_queue_system.h>
#include <ecs/systems/game_state_control_system.h>
#include <ecs/systems/load_map_systems.h>
#include <ecs/systems/phisics_systems.h>
#include <ecs/systems/player_control_systems.h>
#include <ecs/systems/render_hud_systems.h>
#include <ecs/systems/render_objects_systems.h>
#include <my_common_cpp_utils/Logger.h>
#include <utils/file_system.h>
#include <utils/imgui_sdl_RAII.h>
#include <utils/input_event_manager.h>
#include <utils/sdl_RAII.h>

int main(int argc, char* args[])
{
    try
    {
        // Set the current directory to the executable directory.
        std::string execPath = args[0];
        std::string execDir = execPath.substr(0, execPath.find_last_of("\\/"));
        std::filesystem::current_path(execDir);
        MY_LOG_FMT(info, "Set the current directory to: {}", execDir);

        // Initialize the logger with the trace level.
        utils::Logger::getInstance(spdlog::level::info);

        // Check if the map file exists.
        std::string mapPath = "assets\\maps\\map.json";
        if (!std::filesystem::exists(mapPath))
            throw std::runtime_error(MY_FMT("Map file does not found: {}", mapPath));

        // Create an EnTT registry.
        entt::registry registry;

        // Create a game state entity.
        auto& gameState = registry.emplace<GameState>(registry.create());
        gameState.windowOptions.cameraCenter = gameState.windowOptions.windowSize / 2.0f;

        // Create a physics world with gravity and store it in the registry.
        b2Vec2 gravity(0.0f, +9.8f);
        gameState.physicsWorld = std::make_shared<b2World>(gravity);

        // Initialize SDL, create a window and a renderer. Initialize ImGui.
        SDLInitializerRAII sdlInitializer(SDL_INIT_VIDEO);
        SDLWindowRAII window("WOFARES with SDL, ImGui, EnTT, Box2D & GLM", gameState.windowOptions.windowSize);
        SDLRendererRAII renderer(window.get());
        ImGuiSDLRAII imguiSDL(window.get(), renderer.get());

        // Load the map.
        LoadMap(registry, renderer.get(), mapPath);

        // Subscribe all systems that need to handle input events.
        InputEventManager inputEventManager;
        SubscribeCameraControlSystem(registry, inputEventManager);
        SubscribeGameStateControlSystem(registry, inputEventManager);
        SubscribePlayerControlSystem(registry, inputEventManager);

        // Start the game loop.
        Uint32 lastTick = SDL_GetTicks();
        while (!gameState.controlOptions.quit)
        {
            // Calculate delta time.
            Uint32 frameStart = SDL_GetTicks();
            float deltaTime = static_cast<float>(frameStart - lastTick) / 1000.0f;
            lastTick = frameStart;

            if (utils::FileChangedSinceLastCheck(mapPath) || gameState.controlOptions.reloadMap)
            {
                UnloadMap(registry);
                LoadMap(registry, renderer.get(), mapPath);
                gameState.controlOptions.reloadMap = false;
            }

            // Handle input events.
            EventQueueSystem(inputEventManager, deltaTime);

            // Update the physics.
            PhysicsSystem(registry, deltaTime);

            // Render the scene and the HUD.
            imguiSDL.startFrame();
            RenderSystem(registry, renderer.get());
            DrawGridSystem(renderer.get(), gameState);
            RenderHUDSystem(registry, renderer.get());
            imguiSDL.finishFrame();

            // Cap the frame rate.
            Uint32 frameTime = SDL_GetTicks() - frameStart;
            const Uint32 frameDelay = 1000 / gameState.windowOptions.fps;
            if (frameDelay > frameTime)
            {
                SDL_Delay(frameDelay - frameTime);
            }
        }
    }
    catch (const std::runtime_error& e)
    {
        MY_LOG_FMT(warn, "Unhandled exception catched in main: {}", e.what());
        return -1;
    }

    return 0;
}
