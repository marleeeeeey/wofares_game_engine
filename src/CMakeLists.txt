add_executable(wofaers main.cpp)

target_link_libraries(wofaers
    PRIVATE
    glm
    SDL2::SDL2 SDL2::SDL2main
    EnTT::EnTT
    imgui
    nlohmann_json::nlohmann_json
    magic_enum::magic_enum
    spdlog::spdlog
    glm
    my_common_cpp_utils
)

target_include_directories(wofaers
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)
