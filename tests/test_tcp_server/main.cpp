#include <asio.hpp>
#include <iostream>
#include <utils/network/asio/tcp_server.h>

int main(int argc, char* args[])
{
    std::cout << "Server started" << std::endl;

    asio::io_context io_context;
    TcpServer server(io_context, "localhost", "23456");
    server.StartAccept();

    std::atomic<bool> isRunning{true};

    std::thread ioThread([&io_context]() { io_context.run(); });

    std::thread controlThread(
        [&isRunning]()
        {
            std::cout << "Press Ctrl+C to exit...\n";
            std::cin.get();
            isRunning = false;
        });

    while (isRunning)
    {
        auto activeSessions = server.GetActiveSessions();
        std::cout << "Active sessions: " << activeSessions.size() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    io_context.stop();
    if (ioThread.joinable())
    {
        ioThread.join();
    }
    if (controlThread.joinable())
    {
        controlThread.join();
    }

    std::cout << "Server stopped" << std::endl;
}