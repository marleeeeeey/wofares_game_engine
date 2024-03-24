# Create an executable with the collected source files.
add_executable(test_tcp_server
    main.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/network/asio/tcp_json_session.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/network/asio/tcp_server.cpp
)

target_compile_definitions(test_tcp_server
    PRIVATE

    # Add definitions here
)

target_compile_options(test_tcp_server PRIVATE
    -Wall
    -Wextra

    # -Werror
    # -Wpedantic
)

target_link_libraries(test_tcp_server
    PRIVATE
    asio::asio # header-only
    my_common_cpp_utils
)

target_include_directories(test_tcp_server
    PRIVATE
    ./
    ${CMAKE_SOURCE_DIR}/src
)
