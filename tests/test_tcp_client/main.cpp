#include <asio.hpp>
#include <iostream>
#include <utils/network/asio/tcp_client.h>

int main(int argc, char* args[])
{
    std::cout << "Client started" << std::endl;

    asio::io_context io_context;
    asio::io_context::work idleWork(io_context);

    TcpClient client(io_context);
    client.Connect("localhost", "23456");

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