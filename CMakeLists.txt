cmake_minimum_required(VERSION 3.15)
project(ThisGameProject VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_subdirectory(thirdparty/entt)
add_subdirectory(thirdparty/glm)
# add_subdirectory(thirdparty/googletest)
# add_subdirectory(thirdparty/json)
# add_subdirectory(thirdparty/magic_enum)
add_subdirectory(thirdparty/SDL)
# add_subdirectory(thirdparty/spdlog)


####################################################################
############# hack to make imgui avalable via cmake ################
####################################################################
                                                               #####
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui)    #####
                                                               #####
set(IMGUI_SOURCES                                              #####
    ${IMGUI_DIR}/imgui.cpp                                     #####
    ${IMGUI_DIR}/imgui_draw.cpp                                #####
    ${IMGUI_DIR}/imgui_tables.cpp                              #####
    ${IMGUI_DIR}/imgui_widgets.cpp                             #####
    ${IMGUI_DIR}/imgui_demo.cpp                                #####
)                                                              #####
                                                               #####
# Add backends                                                 #####
list(APPEND IMGUI_SOURCES                                      #####
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp                  #####
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp          #####
)                                                              #####
                                                               #####
set(IMGUI_INCLUDE_DIRS                                         #####
    ${IMGUI_DIR}                                               #####
    ${IMGUI_DIR}/backends                                      #####
)                                                              #####
                                                               #####
add_library(imgui STATIC ${IMGUI_SOURCES})                     #####
                                                               #####
target_include_directories(imgui                               #####
    PUBLIC                                                     #####
    ${IMGUI_INCLUDE_DIRS}                                      #####
)                                                              #####
                                                               #####
target_link_libraries(imgui                                    #####
    PRIVATE                                                    #####
    SDL2::SDL2 SDL2::SDL2main                                  #####
)                                                              #####
                                                               #####
####################################################################
####################################################################
####################################################################


add_subdirectory(src)
