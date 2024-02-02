add_executable(YourExecutableName main.cpp)

target_link_libraries(YourExecutableName
    PRIVATE
    glm
    SDL2::SDL2 SDL2::SDL2main
    EnTT::EnTT
)

target_include_directories(YourExecutableName
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)
