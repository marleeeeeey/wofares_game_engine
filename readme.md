# This Game Project

## File Structure

```
ThisGameProject/
├── CMakeLists.txt       # Main CMake file for project configuration
├── thirdparty/          # Third-party libraries and dependencies as git submodules
│   ├── entt/
│   ├── glm/
│   ├── googletest/
│   ├── imgui/
│   ├── imgui-sfml/
│   ├── json/
│   ├── magic_enum/
│   ├── SDL/
│   ├── SFML/
│   └── spdlog/
├── src/                 # Source code of this game
│   ├── CMakeLists.txt   # CMake file for compiling the game
│   ├── main.cpp         # Entry point of the game
│   ├── game/            # Game-specific code
│   │   ├── systems/     # ECS systems
│   │   ├── conponents/  # ECS components
│   │   └── entities/    # Entity definitions
│   └── utils/           # Helper utilities and libraries
├── tools/               # Utilities and tools reusable across projects
│   ├── CMakeLists.txt   # Optionally, if compiling tools as separate executables
│   └── geometry-tools/
├── tests/               # Tests for this project
│   ├── CMakeLists.txt   # CMake file for compiling and running tests
│   └── unit-tests/      # Unit tests
└── docs/                # Documentation of the project
```

- **thirdparty/**: Store all third-party libraries and dependencies here to facilitate their management. Use `add_subdirectory` or `FetchContent` in CMake to integrate these libraries into your project.

- **src/**: The main directory for the source code of your game. Dividing it into subdirectories (`Game`, `Utils`) helps to organize the code by purpose and facilitates its reuse and testing.

- **game/**: Contains code directly related to game logic, including ECS systems, components, and entities. This facilitates the scaling and modification of the game.

- **utils/**: General utilities and libraries that can be used in various parts of your game or even in other projects.

- **tools/**: Store here utilities and tools that might be useful during development but are not part of the final game. This might include scripts for automation, data analysis tools, etc.

- **tests/**: A separate directory for tests keeps the structure clean and simplifies integration with Continuous Integration (CI) systems. Use `googletest` or other frameworks for unit testing.

- **docs/**: Maintaining up-to-date documentation of the project, including architecture, design, API, and guides, is a good practice.

This directory structure is a suggestion and can be adapted to fit the specific needs of your project. The main goal is to maintain consistency and clarity in code organization to ease development and maintenance.
