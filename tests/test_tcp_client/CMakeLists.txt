# Create an executable with the collected source files.
add_executable(test_tcp_client
    main.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/network/asio/tcp_client.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/network/asio/tcp_json_session.cpp
)

target_compile_definitions(test_tcp_client
    PRIVATE

    # Add definitions here
)

target_compile_options(test_tcp_client PRIVATE
    -Wall
    -Wextra

    # -Werror
    # -Wpedantic
)

target_link_libraries(test_tcp_client
    PRIVATE
    asio::asio # header-only
    my_common_cpp_utils
)

target_include_directories(test_tcp_client
    PRIVATE
    ./
    ${CMAKE_SOURCE_DIR}/src
)
