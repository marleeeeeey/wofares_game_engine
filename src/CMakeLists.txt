add_executable(YourExecutableName main.cpp)

target_link_libraries(YourExecutableName
    PRIVATE
    glm
    #SDL2::SDL2 SDL2::SDL2main
    ${SDL2_LIBRARIES}
)

target_include_directories(YourExecutableName
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${SDL2_INCLUDE_DIRS}
)
