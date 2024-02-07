cmake_minimum_required(VERSION 3.15)

project(Wofares VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(GLM_ENABLE_CXX_20 ON CACHE BOOL "" FORCE)
set(GLM_DISABLE_AUTO_DETECTION ON CACHE BOOL "" FORCE)

# Generate compile_commands.json.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(thirdparty/SDL)
add_subdirectory(thirdparty/SDL_image)
add_subdirectory(thirdparty/glm)
add_subdirectory(thirdparty/entt)
add_subdirectory(thirdparty/json)
add_subdirectory(thirdparty/magic_enum)
add_subdirectory(thirdparty/spdlog)
add_subdirectory(thirdparty/my_common_cpp_utils)
add_subdirectory(src)

# TODO: Disable adding the googletest library. This is workaround to make SDL_image linkable via cmake.
# SDL_image\external\libjxl\third_party\googletest conflicts with thirdparty/googletest.
# add_subdirectory(thirdparty/googletest)

# ###################################################################
# ############ hack to make imgui avalable via cmake ################
# ###################################################################
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui)

add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

target_include_directories(imgui
    PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(imgui
    PRIVATE
    SDL2::SDL2
)

# ###################################################################
# ######## end of hack to make imgui avalable via cmake #############
# ###################################################################