#include <asio.hpp>
#include <cstddef>
#include <future>
#include <memory>
#include <nlohmann/json.hpp>
#include <utils/network/asio/tcp_json_session.h>
#include <vector>

class TcpServer
{
    asio::io_context& io_context_;
    asio::ip::tcp::acceptor acceptor_;
    // Use unordered_map to make key always valid opposite the case of using std::vector
    std::unordered_map<size_t, std::shared_ptr<TcpJsonSession>> sessions_;
public:
    TcpServer(asio::io_context& io_context, const std::string& host, const std::string& port);
    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;
public: // Main interface.
    void StartAccept();
    // Send data to all connected clients.
    std::unordered_map<size_t, std::future<void>> Broadcast(const nlohmann::json& j);
    // Receive data from a specific session.
    std::future<nlohmann::json> ReceiveFromSession(size_t sessionIndex);
    // Remove session from the list.
    void CloseSession(size_t sessionIndex);
    std::vector<size_t> GetActiveSessions() const noexcept { return std::vector<size_t>(sessions_.size()); }
};
