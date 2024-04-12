# Recursively collect all .cpp files from the current source directory and subdirectories.
file(GLOB_RECURSE WOFARES_SOURCES "*.cpp")

# Create an executable with the collected source files.
add_executable(wofares ${WOFARES_SOURCES})

target_compile_definitions(wofares
    PRIVATE

    DisableSteamNetworkingSockets # TODO2: Temporary disable SteamNetworkingSockets for LD55.

    # MY_DEBUG # TODO5: Uncomment this to enable extra debug mode.
)

target_compile_options(wofares PRIVATE
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

target_link_libraries(wofares
    PRIVATE

    # vcpkg build libraries:
    box2d::box2d
    EnTT::EnTT
    GameNetworkingSockets::static
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main> $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
    $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>

    # custom build libraries:
    imgui # Because of this package unavailability in linux package manager.
    my_cpp_utils
)

target_include_directories(wofares
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/thirdparty/glob/single_include
)

# copy assets
add_custom_command(TARGET wofares POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/thirdparty/game_assets/wofares/assets"
    "$<TARGET_FILE_DIR:wofares>/assets")

# copy config.json
add_custom_command(TARGET wofares POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/config.json"
    "$<TARGET_FILE_DIR:wofares>/config.json")