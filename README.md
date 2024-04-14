# LD55 (C++ Game Project)

**LD55** - is a platformer game developed for the Ludum Dare 55 game jam.

### Team

- [marleeeeeey](https://github.com/marleeeeeey) - C++ programming, game desing, sound.
- [jsCommander](https://github.com/jsCommander) - game desing, art.

![alt text](docs/LD55_screenshot.png)

### Log

```
Start the jam (I overslept) : 0500 2024-04-13
Start thinking about the game idea : 1200 2024-04-13
Complete repository setup : 1353 2024-04-13
Implement dummy portal catching the player : 1513 2024-04-13
Intergrate evil and portal animations : 1826 2024-04-13
Implement StickyTrap : 0159 2024-04-14
Add Box2dBodyOptions::DestructionPolicy. Support ZOrdering: Background, Interiors, Terrain : 0309 2024-04-14
Portal sticky to trap : 0414 2024-04-14
Me sleeping from 0414 to 0930 : 0930 2024-04-14
Create fast hybrid explosion fragments : 1108 2024-04-14
```

### Assets editing

- Aseprite
  - All frames must have tags. Tags are used in the C++ code.
  - Aseprite must contrain frame with name `Hitbox`.
- Tiled
  - Map may contains layers: `backgound`, `interiors`, `terrain`, `objects` (from bottom to top).

### Platform Support

- This game has been developed, built, and tested solely on **Windows**. Build also works on **Linux(Ubuntu)** but not tested yet.

### Clone the Repository

```
git clone --recursive https://github.com/marleeeeeey/LD55-game.git
```

## Windows

### Prerequisites for Building the Project (Windows)

- CMake.
- Ninja.
- Clang compiler (other compilers may work, but are not officially supported).
- Git (to load submodules).
- Visual Studio Code (recommended for development).
  - Clangd extension (recommended for code analysis).
- Python and 7zip (for packaging game to achive).

### Build, run and debug via VSCode tasks (Windows)

- Open the project folder in VSCode.
- Run task: `003. (Win) Install vcpkg as subfolder`.
- Run task: `050. (WinDebug) + Run`.
- For debugging press `F5`.

### Build, run and debug manually (Windows)

To build LD55 on Windows, it's recommended to obtain the dependencies by using vcpkg. The following instructions assume that you will follow the vcpkg recommendations and install vcpkg as a subfolder. If you want to use "classic mode" or install vcpkg somewhere else, you're on your own.

This project define it's dependences:
1. In a `vcpkg.json` file, and you are pulling in vcpkg's cmake toolchain file.
2. As git submodules in the `thirdparty` directory. Because some of the libraries not available in vcpkg or have an error in the vcpkg port file.

First, we bootstrap a project-specific installation of vcpkg ("manifest mode") in the default location, `<project root>/vcpkg`. From the project root, run these commands:

```
cd LD55-game
git clone https://github.com/microsoft/vcpkg
.\vcpkg\bootstrap-vcpkg.bat
```

Now we ask vcpkg to install of the dependencies for our project, which are described by the file `<project root>/vcpkg.json`.  Note that this step is optional, as cmake will automatically do this.  But here we are doing it in a separate step so that we can isolate any problems, because if problems happen here don't have anything to do with your cmake files.

```
.\vcpkg\vcpkg install --triplet=x64-windows
```

Next build the project files. There are different options for
1. Telling cmake how to integrate with vcpkg: here we use `CMAKE_TOOLCHAIN_FILE` on the command line.
2. Select Ninja project generator.
3. Select Clang compiler.
4. Enable `CMAKE_EXPORT_COMPILE_COMMANDS` to generate a `compile_commands.json` file for clangd.

```
cmake -S . -B build -G "Ninja" -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cd build
cmake --build .
```

Finally, we copy the assets and configuration file to the build directory.

```
cmake -E copy ../config.json ./src/config.json
cmake -E copy_directory ../assets ./src/assets
```

Run the game:

```
src\LD55.exe
```

## Linux

### Prerequisites for Building the Project (Linux)

```bash
sudo apt update
sudo apt-get install python3 clang ninja-build curl zip unzip tar autoconf automake libtool python3-pip cmake
pip install jinja2
```

### Build, run and debug via VSCode tasks (Linux)

- Open the project folder in VSCode.
- Run task: `103. (Lin) Install vcpkg as subfolder`.
- Run task: `150. (LinDebug) + Run`.
- For debugging press `F5`. (TODO3 Implement it later).

### Build, run and debug manually (Linux)

```bash
cd LD55-game
git clone https://github.com/microsoft/vcpkg && ./vcpkg/bootstrap-vcpkg.sh && ./vcpkg/vcpkg install --triplet=x64-linux
cmake -S . -B build -G "Ninja" -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build/debug -- -k 0
cmake -E copy config.json build/debug/src/config.json
cmake -E copy_directory assets build/debug/src/assets
./build/debug/src/LD55
```

## Additional Notes

### File Structure

```

LD55/
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
