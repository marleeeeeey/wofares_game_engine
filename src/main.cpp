#include <ecs/components/game_components.h>
#include <ecs/systems/camera_control_system.h>
#include <ecs/systems/event_queue_system.h>
#include <ecs/systems/game_objects_render_system.h>
#include <ecs/systems/game_state_control_system.h>
#include <ecs/systems/map_loader_system.h>
#include <ecs/systems/phisics_systems.h>
#include <ecs/systems/player_control_systems.h>
#include <ecs/systems/render_hud_systems.h>
#include <ecs/systems/weapon_control_system.h>
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
        gameState.windowOptions.cameraCenterSdl = gameState.windowOptions.windowSize / 2.0f;

        // Create a physics world with gravity and store it in the registry.
        b2Vec2 gravity(0.0f, +9.8f);
        gameState.physicsWorld = std::make_shared<b2World>(gravity);

        // Create a contact listener and subscribe it to the physics world.
        Box2dEnttContactListener contactListener(registry);
        gameState.physicsWorld->SetContactListener(&contactListener);

        // Create a weapon control system and subscribe it to the contact listener.
        WeaponControlSystem weaponControlSystem(registry, contactListener);

        // Initialize SDL, create a window and a renderer. Initialize ImGui.
        SDLInitializerRAII sdlInitializer(SDL_INIT_VIDEO);
        SDLWindowRAII window("WOFARES with SDL, ImGui, EnTT, Box2D & GLM", gameState.windowOptions.windowSize);
        SDLRendererRAII renderer(window.get());
        ImGuiSDLRAII imguiSDL(window.get(), renderer.get());

        // Create an input event manager and an event queue system.
        InputEventManager inputEventManager;
        EventQueueSystem eventQueueSystem(inputEventManager);

        // Subscribe all systems that need to handle input events.
        PlayerControlSystem playerControlSystem(registry, inputEventManager, contactListener);
        CameraControlSystem cameraControlSystem(registry, inputEventManager);
        GameStateControlSystem gameStateControlSystem(registry, inputEventManager);

        // Create a systems with no input events.
        PhysicsSystem physicsSystem(registry);
        GameObjectsRenderSystem gameObjectsRenderSystem(registry, renderer.get());
        HUDRenderSystem hudRenderSystem(registry, renderer.get());
        MapLoaderSystem mapLoaderSystem(registry, renderer.get());

        // Load the map.
        mapLoaderSystem.LoadMap(mapPath);

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
                mapLoaderSystem.UnloadMap();
                mapLoaderSystem.LoadMap(mapPath);
                gameState.controlOptions.reloadMap = false;
            }

            // Handle input events.
            eventQueueSystem.Update(deltaTime);

            // Update the physics and post-physics systems to prepare the render.
            physicsSystem.Update(deltaTime);
            weaponControlSystem.Update(deltaTime);
            cameraControlSystem.Update(deltaTime);

            // Render the scene and the HUD.
            imguiSDL.startFrame();
            gameObjectsRenderSystem.Render();
            hudRenderSystem.Render();
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
