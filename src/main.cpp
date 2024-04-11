#if 0
#include <utils/network/steam_networking_init_RAII.h>
#endif

#include <ecs/systems/animation_update_system.h>
#include <ecs/systems/camera_control_system.h>
#include <ecs/systems/game_objects_render_system.h>
#include <ecs/systems/map_loader_system.h>
#include <ecs/systems/phisics_systems.h>
#include <ecs/systems/player_control_systems.h>
#include <ecs/systems/render_hud_systems.h>
#include <ecs/systems/weapon_control_system.h>
#include <iostream>
#include <magic_enum.hpp>
#include <my_cpp_utils/config.h>
#include <my_cpp_utils/json_utils.h>
#include <my_cpp_utils/logger.h>
#include <utils/RAII/imgui_sdl_RAII.h>
#include <utils/RAII/sdl_RAII.h>
#include <utils/entt_registry_wrapper.h>
#include <utils/factories/objects_factory.h>
#include <utils/file_system.h>
#include <utils/resources/resource_manager.h>
#include <utils/sdl_primitives_renderer.h>
#include <utils/systems/audio_system.h>
#include <utils/systems/event_queue_system.h>
#include <utils/systems/game_state_control_system.h>
#include <utils/systems/input_event_manager.h>
#include <utils/systems/random_event_system.h>
#include <utils/systems/screen_mode_control_system.h>

int main(int argc, char* args[])
{
    try
    {
        // Set the current directory to the executable directory.
        std::string execPath = args[0];
        std::string execDir = execPath.substr(0, execPath.find_last_of("\\/"));
        std::filesystem::current_path(execDir);

        // Set the paths to the configuration and log files.
        std::filesystem::path configFilePath = "config.json";
        std::filesystem::path logFilePath = "logs/wofares.log";

        // Initialize the logger and the configuration.
        utils::Config::InitInstanceFromFile(configFilePath);
        utils::Logger::Init(logFilePath, spdlog::level::info);

        // Log initial information.
        MY_LOG(info, "******************************");
        MY_LOG(info, "**** Wofares game started ****");
        MY_LOG(info, "******************************");
        MY_LOG(info, "Current directory set to: {}", execDir);
        MY_LOG(info, "Config file loaded: {}", configFilePath.string());

#if 0
        // Initialize the SteamNetworkingSockets library.
        SteamNetworkingInitRAII::Options steamNetworkingOptions;
        steamNetworkingOptions.debugSeverity =
            utils::GetConfig<ESteamNetworkingSocketsDebugOutputType, "Networking.debugSeverity">();
        MY_LOG(info, "[SteamNetworking] debugSeverity: {}", steamNetworkingOptions.debugSeverity);
        SteamNetworkingInitRAII steamNetworkingInitRAII(steamNetworkingOptions);
        SteamNetworkingInitRAII::SetDebugCallback(
            []([[maybe_unused]] ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg)
            { MY_LOG(info, "[SteamNetworking] {}", pszMsg); });
#endif

        // Create an EnTT registry.
        entt::registry registry;
        EnttRegistryWrapper registryWrapper(registry);

        // Create a game state entity.
        auto& gameOptions = registry.emplace<GameOptions>(
            registryWrapper.Create("GameOptions"), utils::GetConfig<GameOptions, "GameOptions">());

        // Create a physics world with gravity and store it in the registry.
        b2Vec2 gravity(0.0f, +9.8f);
        gameOptions.physicsWorld = std::make_shared<b2World>(gravity);

        // Create a contact listener and subscribe it to the physics world.
        Box2dEnttContactListener contactListener(registryWrapper);
        gameOptions.physicsWorld->SetContactListener(&contactListener);

        // Initialize SDL, create a window and a renderer. Initialize ImGui.
        SDLInitializerRAII sdlInitializer(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
        SDLAudioInitializerRAII sdlAudioInitializer;
        SDLWindowRAII window("WOFARES with SDL, ImGui, EnTT, Box2D & GLM", gameOptions.windowOptions.windowSize);
        SDLRendererRAII renderer(window.get());
        ImGuiSDLRAII imguiSDL(window.get(), renderer.get());

        std::filesystem::path assetsSettingsFilePath = "assets/assets_settings.json";
        auto assetsSettingsJson = utils::LoadJsonFromFile(assetsSettingsFilePath);
        MY_LOG(info, "Assets settings loaded: {}", assetsSettingsFilePath.string());
        ResourceManager resourceManager(renderer.get(), assetsSettingsJson);
        AudioSystem audioSystem(resourceManager);
        audioSystem.PlayMusic("background_music");

        ObjectsFactory objectsFactory(registryWrapper, resourceManager);

        // Create a weapon control system and subscribe it to the contact listener.
        WeaponControlSystem weaponControlSystem(registryWrapper, contactListener, audioSystem, objectsFactory);

        // Create an input event manager and an event queue system.
        InputEventManager inputEventManager;
        EventQueueSystem eventQueueSystem(inputEventManager);

        // Subscribe all systems that need to handle input events.
        PlayerControlSystem playerControlSystem(registryWrapper, inputEventManager, contactListener, objectsFactory);
        CameraControlSystem cameraControlSystem(registryWrapper.GetRegistry(), inputEventManager);
        GameStateControlSystem gameStateControlSystem(registryWrapper.GetRegistry(), inputEventManager);

        // Create a systems with no input events.
        SdlPrimitivesRenderer primitivesRenderer(registryWrapper.GetRegistry(), renderer.get(), resourceManager);
        PhysicsSystem physicsSystem(registryWrapper);
        RandomEventSystem randomEventSystem(registryWrapper.GetRegistry(), audioSystem);
        GameObjectsRenderSystem gameObjectsRenderSystem(
            registryWrapper.GetRegistry(), renderer.get(), resourceManager, primitivesRenderer);
        HUDRenderSystem hudRenderSystem(registryWrapper.GetRegistry(), renderer.get());
        MapLoaderSystem mapLoaderSystem(registryWrapper, resourceManager);

        // Auxiliary systems.
        ScreenModeControlSystem screenModeControlSystem(inputEventManager, window);

        // Load the map.
        auto level1 = resourceManager.GetTiledLevel("level1");
        mapLoaderSystem.LoadMap(level1);

        AnimationUpdateSystem animationUpdateSystem(registryWrapper.GetRegistry(), resourceManager);

        // Start the game loop.
        Uint32 lastTick = SDL_GetTicks();
        while (!gameOptions.controlOptions.quit)
        {
            // Calculate delta time.
            Uint32 frameStart = SDL_GetTicks();
            float deltaTime = static_cast<float>(frameStart - lastTick) / 1000.0f;
            lastTick = frameStart;

            if (utils::FileChangedSinceLastCheck(level1.tiledMapPath) || gameOptions.controlOptions.reloadMap)
            {
                mapLoaderSystem.UnloadMap();
                mapLoaderSystem.LoadMap(level1);
                gameOptions.controlOptions.reloadMap = false;
            }

            // Handle input events.
            eventQueueSystem.Update(deltaTime);

            randomEventSystem.Update(deltaTime);

            // Update the physics and post-physics systems to prepare the render.
            physicsSystem.Update(deltaTime);
            playerControlSystem.Update(deltaTime);
            weaponControlSystem.Update(deltaTime);
            cameraControlSystem.Update(deltaTime);

            // Update animation.
            animationUpdateSystem.Update(deltaTime);

            // Render the scene and the HUD.
            imguiSDL.startFrame();
            gameObjectsRenderSystem.Render();
            hudRenderSystem.Render();
            imguiSDL.finishFrame();

            // Cap the frame rate.
            Uint32 frameTime = SDL_GetTicks() - frameStart;
            const Uint32 frameDelay = 1000 / gameOptions.windowOptions.fps;
            if (frameDelay > frameTime)
            {
                SDL_Delay(frameDelay - frameTime);
            }
        }

        registryWrapper.LogAllEntitiesByTheirNames();
    }
    catch (const std::runtime_error& e)
    {
        // This line is needed to log exceptions from the logger. Because the logger is used in the catch block.
        std::cout << "Unhandled exception catched in main: " << e.what() << std::endl;

        MY_LOG(warn, "Unhandled exception catched in main: {}", e.what());
        return -1;
    }

    return 0;
}
