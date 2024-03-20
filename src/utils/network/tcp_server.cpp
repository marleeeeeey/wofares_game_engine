#include "tcp_server.h"

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
                auto session = std::make_shared<TcpJsonSession>(std::move(socket));
                sessions_.push_back(session);

                // TODO0: Notify about new connection.
            }

            // Plan to accept the next connection.
            // This it not recursive, it is a new call to start_accept.
            StartAccept();
        });
}

std::vector<std::future<void>> TcpServer::Broadcast(const nlohmann::json& j)
{
    if (sessions_.empty())
    {
        throw std::runtime_error("[TcpServer::Broadcast] No clients connected");
    }

    std::vector<std::future<void>> futures;
    for (auto& session : sessions_)
    {
        futures.push_back(session->Send(j));
    }
    return futures;
}

void TcpServer::ReceiveFromSession(size_t session_index)
{
    if (session_index < sessions_.size())
    {
        auto session = sessions_[session_index];
        auto future = session->Receive();

        // TODO0: Handle the future with the received data.
    }
    else
    {
        throw std::runtime_error("[TcpServer::ReceiveFromSession] Invalid session index");
    }
}
