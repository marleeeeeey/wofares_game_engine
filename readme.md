# Wofaers (C++ Game Project)

## File Structure

```
ThisGameProject/
├── CMakeLists.txt       # Main CMake file for project configuration.
├── thirdparty/          # Third-party libraries and dependencies as git submodules.
│   ├── SDL/
│   ├── glm/
│   ├── entt/
│   └── ...
├── src/                 # Source code of this game.
│   ├── CMakeLists.txt   # CMake file for compiling the game.
│   ├── main.cpp         # Entry point of the game.
│   ├── ecs/             # ECS items.
│   │   ├── systems/     # ECS systems.
│   │   ├── conponents/  # ECS components.
│   │   └── entities/    # ECS entities definitions.
│   └── utils/           # Helper utilities and libraries for this game only.
├── tests/               # Tests for this project.
│   ├── CMakeLists.txt   # CMake file for compiling and running tests.
│   └── unit-tests/      # Unit tests.
└── docs/                # Documentation of the project.
```
