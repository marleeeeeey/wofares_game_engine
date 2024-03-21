#pragma once
#include <asio.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <utils/network/tcp_json_session.h>

class TcpClient
{
    asio::io_context& io_context;
    std::shared_ptr<TcpJsonSession> tcpJsonSession;
public:
    TcpClient(asio::io_context& io_context);
    TcpClient(const TcpClient&) = delete;
    TcpClient& operator=(const TcpClient&) = delete;
public: // Main interface.
    void Connect(const std::string& host, const std::string& port);
    void Disconnect();
    std::future<void> Send(const nlohmann::json& j);
    std::future<nlohmann::json> Receive();
};
