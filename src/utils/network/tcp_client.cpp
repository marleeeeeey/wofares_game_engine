#include "tcp_client.h"
#include <memory>

TcpClient::TcpClient(asio::io_context& io_context) : io_context(io_context)
{}

void TcpClient::Connect(const std::string& host, const std::string& port)
{
    asio::ip::tcp::resolver resolver(io_context);
    asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, port);
    asio::ip::tcp::socket socket(io_context);
    asio::connect(socket, endpoints);
    tcpJsonSession = std::make_shared<TcpJsonSession>(std::move(socket));
}

std::future<void> TcpClient::Send(const nlohmann::json& j)
{
    if (!tcpJsonSession)
    {
        throw std::runtime_error("[TcpClient::Send] Client not connected");
    }

    return tcpJsonSession->Send(j);
}

std::future<nlohmann::json> TcpClient::Receive()
{
    if (!tcpJsonSession)
    {
        throw std::runtime_error("[TcpClient::Receive] Client not connected");
    }

    return tcpJsonSession->Receive();
}
