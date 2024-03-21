#pragma once
#include <asio.hpp>
#include <nlohmann/json.hpp>

// std::enable_shared_from_this<TcpJsonSession> used to allow shared_from_this() in async handlers.
// It is used to keep the TcpJsonSession object alive while asynchronous operations are in progress.
class TcpJsonSession : std::enable_shared_from_this<TcpJsonSession>
{
    asio::ip::tcp::socket socket_;
public:
    TcpJsonSession(asio::ip::tcp::socket socket);
    ~TcpJsonSession();
public: // Main interface.
    std::future<void> Send(const nlohmann::json& j);
    std::future<nlohmann::json> Receive();
    void Close();
};
