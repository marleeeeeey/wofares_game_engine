#include <asio.hpp>
#include <future>
#include <memory>
#include <nlohmann/json.hpp>
#include <utils/network/tcp_json_session.h>

class TcpServer
{
    asio::io_context& io_context_;
    asio::ip::tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<TcpJsonSession>> sessions_;
public:
    TcpServer(asio::io_context& io_context, const std::string& host, const std::string& port);

    void StartAccept();

    // Send data to all connected clients.
    std::vector<std::future<void>> Broadcast(const nlohmann::json& j);

    // Receive data from a specific session. The session_index is the index of the session in the sessions_ vector.
    // Possible to run this method for several sessions one after another without waiting for the previous one session
    // to finish.
    void ReceiveFromSession(size_t session_index);
};
