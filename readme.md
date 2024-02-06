# Wofares (C++ Game Project)

## How to build

```
mkdir build
cd build
cmake -G "Ninja"-DCMAKE_CXX_COMPILER=clang++ ..
ninja
```

## File Structure

```
ThisGameProject/
├── CMakeLists.txt
├── thirdparty/
│   ├── SDL/
│   ├── glm/
│   ├── entt/
│   └── ...
├── src/
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── ecs/
│   │   ├── systems/
│   │   └── conponents/
│   └── utils/
├── tests/
│   ├── CMakeLists.txt
│   └── *.cpp
├── assets
│   ├── images
│   │   └── *.png
│   ├── maps
│   │   └── *.json
│   ├── sounds
│   │   └── *.wav
│   └── fonts
│       └── *.ttf
└── docs/
    └── *.*
```
