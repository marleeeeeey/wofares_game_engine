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
│   ├── graphics/
│   └── utils/
├── tests/
│   ├── CMakeLists.txt
│   └── unit-tests/
└── docs/
```
