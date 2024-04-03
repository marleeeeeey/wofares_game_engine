#include "tcp_server.h"
#include <cstddef>

TcpServer::TcpServer(asio::io_context& io_context, const std::string& host, const std::string& port)
  : io_context_(io_context), acceptor_(asio::make_strand(io_context))
{
    asio::ip::tcp::resolver resolver(io_context_);
    auto endpoints = resolver.resolve(host, port);
    acceptor_.open(endpoints->endpoint().protocol());
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoints->endpoint());
    acceptor_.listen();
}

void TcpServer::StartAccept()
{
    acceptor_.async_accept(
        [this](std::error_code ec, asio::ip::tcp::socket socket)
        {
            if (!ec)
            {
                // TODO3: size_t type has a very big limit, but it have. Think how to reuse disconnected ids.
                static size_t newSessionId = 0;
                newSessionId++;
                auto session = std::make_shared<TcpJsonSession>(std::move(socket));
                sessions_[newSessionId] = session;

                // TODO0: Notify about new connection.
            }

            // Plan to accept the next connection.
            // This it not recursive, it is a new call to start_accept.
            StartAccept();
        });
}

std::unordered_map<size_t, std::future<void>> TcpServer::Broadcast(const nlohmann::json& j)
{
    if (sessions_.empty())
    {
        throw std::runtime_error("[TcpServer::Broadcast] No clients connected");
    }

    std::unordered_map<size_t, std::future<void>> futures;
    for (auto& [id, session] : sessions_)
    {
        futures[id] = session->Send(j);
    }
    return futures;
}

// Possible to run this method for several sessions one after another
// without waiting for the previous one session to finish.
std::future<nlohmann::json> TcpServer::ReceiveFromSession(size_t sessionIndex)
{
    if (!sessions_.contains(sessionIndex))
    {
        throw std::runtime_error("[TcpServer::ReceiveFromSession] Invalid session index");
    }

    auto session = sessions_[sessionIndex];
    auto future = session->Receive();
    return future;
}

void TcpServer::CloseSession(size_t sessionIndex)
{
    if (!sessions_.contains(sessionIndex))
    {
        throw std::runtime_error("[TcpServer::CloseSession] Invalid session index");
    }

    sessions_.erase(sessionIndex); // It close the socket on object destruction
}
