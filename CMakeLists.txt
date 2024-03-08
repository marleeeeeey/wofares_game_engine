cmake_minimum_required(VERSION 3.15)

project(Wofares VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(GLM_ENABLE_CXX_20 ON CACHE BOOL "" FORCE)
set(GLM_DISABLE_AUTO_DETECTION ON CACHE BOOL "" FORCE)

# Generate compile_commands.json.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Getting vcpkg root dir.
message(STATUS "CMAKE_TOOLCHAIN_FILE is set to ${CMAKE_TOOLCHAIN_FILE}")
string(REGEX REPLACE "/scripts/buildsystems/vcpkg.cmake$" "" VCPKG_ROOT_DIR "${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "VCPKG root dir: ${VCPKG_ROOT_DIR}")

# TODO5: here is a hack to make SDL_image linkable via cmake. Fix it and make cross-platform.
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${VCPKG_ROOT_DIR}/installed/x64-windows/share/SDL2_mixer")
find_package(SDL2_mixer CONFIG REQUIRED)

add_subdirectory(thirdparty/SDL)
add_subdirectory(thirdparty/SDL_image)
add_subdirectory(thirdparty/glm)
add_subdirectory(thirdparty/entt)
add_subdirectory(thirdparty/json)
add_subdirectory(thirdparty/magic_enum)
add_subdirectory(thirdparty/spdlog)
add_subdirectory(thirdparty/my_common_cpp_utils)
add_subdirectory(src)

# TODO5: Disable adding the googletest library. This is workaround to make SDL_image linkable via cmake.
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

# ###################################################################
# ############ hack to make box2d avalable via cmake ################
# ###################################################################
set(BOX2D_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/box2d)

add_library(box2d STATIC
    ${BOX2D_DIR}/src/collision/b2_broad_phase.cpp
    ${BOX2D_DIR}/src/collision/b2_chain_shape.cpp
    ${BOX2D_DIR}/src/collision/b2_circle_shape.cpp
    ${BOX2D_DIR}/src/collision/b2_collide_circle.cpp
    ${BOX2D_DIR}/src/collision/b2_collide_edge.cpp
    ${BOX2D_DIR}/src/collision/b2_collide_polygon.cpp
    ${BOX2D_DIR}/src/collision/b2_collision.cpp
    ${BOX2D_DIR}/src/collision/b2_distance.cpp
    ${BOX2D_DIR}/src/collision/b2_dynamic_tree.cpp
    ${BOX2D_DIR}/src/collision/b2_edge_shape.cpp
    ${BOX2D_DIR}/src/collision/b2_polygon_shape.cpp
    ${BOX2D_DIR}/src/collision/b2_time_of_impact.cpp
    ${BOX2D_DIR}/src/common/b2_block_allocator.cpp
    ${BOX2D_DIR}/src/common/b2_draw.cpp
    ${BOX2D_DIR}/src/common/b2_math.cpp
    ${BOX2D_DIR}/src/common/b2_settings.cpp
    ${BOX2D_DIR}/src/common/b2_stack_allocator.cpp
    ${BOX2D_DIR}/src/common/b2_timer.cpp
    ${BOX2D_DIR}/src/dynamics/b2_body.cpp
    ${BOX2D_DIR}/src/dynamics/b2_chain_circle_contact.cpp
    ${BOX2D_DIR}/src/dynamics/b2_chain_circle_contact.h
    ${BOX2D_DIR}/src/dynamics/b2_chain_polygon_contact.cpp
    ${BOX2D_DIR}/src/dynamics/b2_chain_polygon_contact.h
    ${BOX2D_DIR}/src/dynamics/b2_circle_contact.cpp
    ${BOX2D_DIR}/src/dynamics/b2_circle_contact.h
    ${BOX2D_DIR}/src/dynamics/b2_contact.cpp
    ${BOX2D_DIR}/src/dynamics/b2_contact_manager.cpp
    ${BOX2D_DIR}/src/dynamics/b2_contact_solver.cpp
    ${BOX2D_DIR}/src/dynamics/b2_contact_solver.h
    ${BOX2D_DIR}/src/dynamics/b2_distance_joint.cpp
    ${BOX2D_DIR}/src/dynamics/b2_edge_circle_contact.cpp
    ${BOX2D_DIR}/src/dynamics/b2_edge_circle_contact.h
    ${BOX2D_DIR}/src/dynamics/b2_edge_polygon_contact.cpp
    ${BOX2D_DIR}/src/dynamics/b2_edge_polygon_contact.h
    ${BOX2D_DIR}/src/dynamics/b2_fixture.cpp
    ${BOX2D_DIR}/src/dynamics/b2_friction_joint.cpp
    ${BOX2D_DIR}/src/dynamics/b2_gear_joint.cpp
    ${BOX2D_DIR}/src/dynamics/b2_island.cpp
    ${BOX2D_DIR}/src/dynamics/b2_island.h
    ${BOX2D_DIR}/src/dynamics/b2_joint.cpp
    ${BOX2D_DIR}/src/dynamics/b2_motor_joint.cpp
    ${BOX2D_DIR}/src/dynamics/b2_mouse_joint.cpp
    ${BOX2D_DIR}/src/dynamics/b2_polygon_circle_contact.cpp
    ${BOX2D_DIR}/src/dynamics/b2_polygon_circle_contact.h
    ${BOX2D_DIR}/src/dynamics/b2_polygon_contact.cpp
    ${BOX2D_DIR}/src/dynamics/b2_polygon_contact.h
    ${BOX2D_DIR}/src/dynamics/b2_prismatic_joint.cpp
    ${BOX2D_DIR}/src/dynamics/b2_pulley_joint.cpp
    ${BOX2D_DIR}/src/dynamics/b2_revolute_joint.cpp
    ${BOX2D_DIR}/src/dynamics/b2_weld_joint.cpp
    ${BOX2D_DIR}/src/dynamics/b2_wheel_joint.cpp
    ${BOX2D_DIR}/src/dynamics/b2_world.cpp
    ${BOX2D_DIR}/src/dynamics/b2_world_callbacks.cpp
    ${BOX2D_DIR}/src/rope/b2_rope.cpp
)

target_include_directories(box2d
    PUBLIC
    ${BOX2D_DIR}/include
)

# ###################################################################
# ######## end of hack to make box2d avalable via cmake #############
# ###################################################################