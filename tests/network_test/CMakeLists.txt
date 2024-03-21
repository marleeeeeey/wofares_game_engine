# Create an executable with the collected source files.
add_executable(network_test
    main.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/network/tcp_client.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/network/tcp_json_session.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/network/tcp_server.cpp
)

target_compile_definitions(network_test
    PRIVATE

    # Add definitions here
)

target_compile_options(network_test PRIVATE
    -Wall
    -Wextra

    # -Werror
    # -Wpedantic
)

target_link_libraries(network_test
    PRIVATE
    asio::asio # header-only
)

target_include_directories(network_test
    PRIVATE
    ./
    ${CMAKE_SOURCE_DIR}/src
)
