# Recursively collect all .cpp files from the current source directory and subdirectories.
file(GLOB_RECURSE LD55_Hungry_Portals_SOURCES "*.cpp")

# Create an executable with the collected source files.
add_executable(LD55_Hungry_Portals ${LD55_Hungry_Portals_SOURCES})

if(EMSCRIPTEN)
    # To copy the assets and config.json to the WASM's virtual filesystem.
    set(EM_LINK_FLAGS "--preload-file \"${CMAKE_SOURCE_DIR}/assets\"@/assets")
    set(EM_LINK_FLAGS "${EM_LINK_FLAGS} --preload-file \"${CMAKE_SOURCE_DIR}/config.json\"@/config.json")

    # To increase memory limit for the game.
    set(EM_LINK_FLAGS "${EM_LINK_FLAGS} -s INITIAL_MEMORY=33554432")
    set(EM_LINK_FLAGS "${EM_LINK_FLAGS} -s ALLOW_MEMORY_GROWTH=1")
    set(EM_LINK_FLAGS "${EM_LINK_FLAGS} -s ABORTING_MALLOC=0")

    # Set the flags.
    set_target_properties(LD55_Hungry_Portals PROPERTIES LINK_FLAGS "${EM_LINK_FLAGS}")

    # Enable the HTML output.
    set_target_properties(LD55_Hungry_Portals PROPERTIES SUFFIX ".html")
endif()

target_compile_definitions(LD55_Hungry_Portals
    PRIVATE

    DisableSteamNetworkingSockets # TODO2: Temporary disable SteamNetworkingSockets for LD55_Hungry_Portals.

    # MY_DEBUG # TODO5: Uncomment this to enable extra debug mode.
)

target_compile_options(LD55_Hungry_Portals PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wpedantic

    # ***********************************************
    # *** Some warnings from HolyBlackCat's code. ***
    # ***********************************************
    -Wdeprecated # search for deprecated functions.
    -Wextra-semi # search for unused `;` in the code.
    -Wimplicit-fallthrough # search for missing `break` and `cases` in switch cases.

    # -Wconversion # search for implicit conversions. Hard to fix everything if not applying from scratch.

    # ******************************************
    # *** Disable some warnings temporarily. ***
    # ******************************************
    $<$<CXX_COMPILER_ID:Clang>:-Wno-deprecated-declarations> # TODO4. Remove this. glob/glob.hpp:173:28: warning: 'getenv' is deprecated
)

target_link_libraries(LD55_Hungry_Portals
    PRIVATE

    # vcpkg build libraries:
    box2d::box2d
    EnTT::EnTT

    # GameNetworkingSockets::static # TODO2: Add as submodule.
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main> $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
    $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>
    SDL2::SDL2_gfx

    # custom build libraries:
    imgui # Because of this package unavailability in linux package manager.
    my_cpp_utils
)

target_include_directories(LD55_Hungry_Portals
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/thirdparty/glob/single_include
)

# copy assets
add_custom_command(TARGET LD55_Hungry_Portals POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:LD55_Hungry_Portals>/assets")

# copy config.json
add_custom_command(TARGET LD55_Hungry_Portals POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/config.json"
    "$<TARGET_FILE_DIR:LD55_Hungry_Portals>/config.json")