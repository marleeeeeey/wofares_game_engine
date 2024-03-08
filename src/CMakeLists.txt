# Recursively collect all .cpp files from the current source directory and subdirectories.
file(GLOB_RECURSE SOURCES "*.cpp")

# Create an executable with the collected source files.
add_executable(wofares ${SOURCES})

target_compile_definitions(wofares
    PRIVATE

    # MY_DEBUG # TODO5: Uncomment this to enable extra debug mode.
)

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
    box2d
    my_common_cpp_utils
    SDL2_mixer::SDL2_mixer
)

target_include_directories(wofares
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/thirdparty/glob/single_include
)

# Helper to copy required DLLs and debug symbols to the output directory.
function(copy_dynamic_lib target_name dll_dir dll_name)
    # Determine the appropriate DLL and PDB names
    if(WIN32)
        if(CMAKE_BUILD_TYPE MATCHES Debug)
            set(dll_file "${dll_name}d.dll")
            set(pdb_file "${dll_name}d.pdb")
        else()
            set(dll_file "${dll_name}.dll")
            set(pdb_file "${dll_name}.pdb")
        endif()
    elseif(APPLE)
        set(dll_file "lib${dll_name}.dylib")
    else()
        set(dll_file "lib${dll_name}.so")
    endif()

    # Copy dynamic library.
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${dll_dir}/${dll_file}"
        "$<TARGET_FILE_DIR:${target_name}>")

    if(WIN32)
        # Copy debug symbols.
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${dll_dir}/${pdb_file}"
            "$<TARGET_FILE_DIR:${target_name}>")
    endif()
endfunction()

# C:\dev\vcpkg\installed\x64-windows\debug\bin\SDL2_mixerd.dll
# C:\dev\vcpkg\installed\x64-windows\debug\bin\SDL2_mixerd.pdb
function(copy_lib_from_vcpkg target_name dll_name)
    # Set the path to the appropriate DLL directory based on the build type.
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        set(dll_dir "${VCPKG_ROOT_DIR}/installed/x64-windows/debug/bin")
    else()
        set(dll_dir "${VCPKG_ROOT_DIR}/installed/x64-windows/bin")
    endif()

    # Determine the appropriate DLL and PDB names
    if(WIN32)
        if(CMAKE_BUILD_TYPE MATCHES Debug)
            set(dll_file "${dll_name}d.dll")
            set(pdb_file "${dll_name}d.pdb")
        else()
            set(dll_file "${dll_name}.dll")
            set(pdb_file "${dll_name}.pdb")
        endif()
    elseif(APPLE)
        set(dll_file "lib${dll_name}.dylib")
    else()
        set(dll_file "lib${dll_name}.so")
    endif()

    # Copy the SDL2_mixer dynamic library.
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${dll_dir}/${dll_file}"
        "$<TARGET_FILE_DIR:${target_name}>")

    if(WIN32)
        # Copy the SDL2_mixer debug symbols.
        if(CMAKE_BUILD_TYPE MATCHES Debug)
            add_custom_command(TARGET ${target_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${dll_dir}/${pdb_file}"
                "$<TARGET_FILE_DIR:${target_name}>")
        endif()
    endif()
endfunction()

copy_dynamic_lib(wofares "${CMAKE_BINARY_DIR}/thirdparty/SDL" "SDL2")
copy_dynamic_lib(wofares "${CMAKE_BINARY_DIR}/thirdparty/spdlog" "spdlog")
copy_dynamic_lib(wofares "${CMAKE_BINARY_DIR}/thirdparty/SDL_image" "SDL2_image")

copy_lib_from_vcpkg(wofares "SDL2_mixer")

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