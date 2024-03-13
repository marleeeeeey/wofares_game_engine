# WOFARES (C++ Game Project)

**WOFARES** - **W**orld **OF** squ**ARES** is a platformer game innovatively designed around the concept of level destruction. It features a unique mechanism where tiles shatter into pieces, with fragments dispersing realistically thanks to the integration of the Box2D physics engine. This game is envisioned to support cooperative multiplayer gameplay and single-player mode.

At the core of WOFARES's engine lies a robust foundation constructed using C++ and leveraging libraries such as SDL, EnTT, gml, imgui, and Box2D. A pivotal aspect of the game's design is the extensive application of the Entity Component System (ECS) pattern. This architectural approach significantly reduces component coupling and simplifies engine maintenance, ensuring a seamless and immersive gaming experience.

![alt text](docs/wofares_screenshot.png)

### Platform Support

- This game has been developed, built, and tested solely on **Windows**. While cross-platform libraries suggest the potential for broader compatibility, official support for platforms other than Windows is not currently offered.

### Prerequisites for Building the Project

- CMake.
- Ninja.
- Clang compiler (other compilers may work, but are not officially supported).
- Git (to load submodules).
- Visual Studio Code (recommended for development).
  - Clangd extension (recommended for code analysis). Project is configured to generate `compile_commands.json` for Clangd.

### Build and run from Visual Studio Code

- `git clone --recursive https://github.com/marleeeeeey/wofares-game.git` to clone the repository and its submodules.
- Open the project folder in VSCode.
- Press `F5` to build and run the game. This action triggers the CMake configuration, project build, copy assets and game launch.

### Build and run from console

```cmd
git clone --recursive https://github.com/marleeeeeey/wofares-game.git
cd wofares-game
git submodule update --init --recursive
mkdir build
cd build
cmake -G "Ninja" DCMAKE_CXX_COMPILER=clang++ -B . -S ..
cmake --build .
# copy ${workspaceFolder}/config.json ${workspaceFolder}/build/src/config.json
# copy_directory ${workspaceFolder}/thirdparty/game_assets/wofares/assets ${workspaceFolder}/build/src/assets
src/wofares.exe
```

### File Structure

```cmd
wofares/
├── CMakeLists.txt          # Main CMakeLists file for the project. Used to search dependencies and set up the project.
├── thirdparty/
│   ├── SDL/
│   ├── glm/
│   ├── entt/
│   └── ...
├── src/
│   ├── CMakeLists.txt      # Main CMakeLists file for the game. Used to determine build targets and copy assets.
│   ├── main.cpp            # Entry point of the game.
│   ├── ecs/                # Entity Component System (ECS) items.
│   │   ├── systems/        # ECS systems.
│   │   └── conponents/     # ECS components.
│   └── utils/              # Utility classes - theoretically, may be moved to a separate library.
│       ├── factories/      # Factories for creating game objects.
│       ├── RAII/           # RAII wrappers for third-party libraries.
│       ├── resources/      # Resource management, like loading textures, sounds, etc.
│       └── systems/        # More widely used systems that has less dependencies from ECS and game specific code.
└── docs/                   # Include media files for general documentation.
    └── *.*
```

### Video log

2024-02-26 https://youtu.be/8OZJOwcWsZs
