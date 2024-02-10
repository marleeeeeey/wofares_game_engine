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
    box2d
    my_common_cpp_utils
)

target_include_directories(wofares
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src
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

copy_dynamic_lib(wofares "${CMAKE_BINARY_DIR}/thirdparty/SDL" "SDL2")
copy_dynamic_lib(wofares "${CMAKE_BINARY_DIR}/thirdparty/spdlog" "spdlog")
copy_dynamic_lib(wofares "${CMAKE_BINARY_DIR}/thirdparty/SDL_image" "SDL2_image")