cmake_minimum_required(VERSION 3.15)
project(ThisGameProject VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add_subdirectory(thirdparty/entt)
add_subdirectory(thirdparty/glm)
# add_subdirectory(thirdparty/googletest)
# add_subdirectory(thirdparty/imgui)
# add_subdirectory(thirdparty/imgui-sfml)
# add_subdirectory(thirdparty/json)
# add_subdirectory(thirdparty/magic_enum)
add_subdirectory(thirdparty/SDL)
# add_subdirectory(thirdparty/SFML)
# add_subdirectory(thirdparty/spdlog)

add_subdirectory(src)
