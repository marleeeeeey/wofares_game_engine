#include <asio.hpp>
#include <iostream>
#include <utils/network/tcp_client.h>
#include <utils/network/tcp_server.h>

void StartServerLoop()
{
    std::cout << "Server started" << std::endl;

    asio::io_context io_context;
    TcpServer server(io_context, "localhost", "12345");
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

void StartClientLoop()
{
    std::cout << "Client started" << std::endl;

    asio::io_context io_context;
    asio::io_context::work idleWork(io_context);

    TcpClient client(io_context);
    client.Connect("localhost", "12345");

    std::thread ioThread([&]() { io_context.run(); });

    while (true)
    {
        std::string message;
        std::cout << "Enter message: ";
        std::getline(std::cin, message);
        if (message.empty())
        {
            break;
        }

        client.Send(message);
    }

    io_context.stop();
    if (ioThread.joinable())
    {
        ioThread.join();
    }
}

int main(int argc, char* args[])
{
    std::cout << "Network test" << std::endl;

    if (argc < 2)
    {
        std::cout << "Usage: " << args[0] << " <server|client>" << std::endl;
        return 1;
    }

    if (std::string(args[1]) == "server")
    {
        StartServerLoop();
    }
    else if (std::string(args[1]) == "client")
    {
        StartClientLoop();
    }
    else
    {
        std::cout << "Usage: " << args[0] << " <server|client>" << std::endl;
        return 1;
    }

    return 0;
}