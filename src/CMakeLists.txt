# Recursively collect all .cpp files from the current source directory and subdirectories.
file(GLOB_RECURSE SOURCES "*.cpp")

# Create an executable with the collected source files.
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

# Helper to copy any required DLLs to the output directory.
function(copy_dll target_name dll_name)
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/thirdparty/${dll_name}"
        "$<TARGET_FILE_DIR:${target_name}>")
endfunction()

# Define dependent DLLs name based on the platform.
if(WIN32)
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        set(SDL_DLL_NAME SDL2d.dll)
        set(SPD_LOG_DLL_NAME spdlogd.dll)
    else()
        set(SDL_DLL_NAME SDL2.dll)
        set(SPD_LOG_DLL_NAME spdlog.dll)
    endif()
elseif(APPLE)
    set(SDL_DLL_NAME libSDL2.dylib)
    set(SPD_LOG_DLL_NAME libspdlog.dylib)
else()
    set(SDL_DLL_NAME libSDL2.so)
    set(SPD_LOG_DLL_NAME libspdlog.so)
endif()

# Copy the required DLLs to the output directory.
copy_dll(wofares "SDL/${SDL_DLL_NAME}")
copy_dll(wofares "spdlog/${SPD_LOG_DLL_NAME}")
