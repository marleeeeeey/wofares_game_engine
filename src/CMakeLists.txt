# Recursively collect all .cpp files from the current source directory and subdirectories
file(GLOB_RECURSE SOURCES "*.cpp")

# Create an executable with the collected source files
add_executable(wofares ${SOURCES})

target_link_libraries(wofares
    PRIVATE
    glm
    SDL2::SDL2 SDL2::SDL2main
    SDL2_image
    EnTT::EnTT
    imgui
    nlohmann_json::nlohmann_json
    magic_enum::magic_enum
    spdlog::spdlog
    glm
    my_common_cpp_utils
)

target_include_directories(wofares
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)

# Define the SDL2 DLL name cross-platform.
if(WIN32)
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        set(SDL_DLL_NAME SDL2d.dll)
    else()
        set(SDL_DLL_NAME SDL2.dll)
    endif()
elseif(APPLE)
    set(SDL_DLL_NAME libSDL2.dylib)
else()
    set(SDL_DLL_NAME libSDL2.so)
endif()

# Copy the SDL2 DLL to the build directory after building.
add_custom_command(TARGET wofares POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/thirdparty/SDL/${SDL_DLL_NAME}"
    "$<TARGET_FILE_DIR:wofares>")