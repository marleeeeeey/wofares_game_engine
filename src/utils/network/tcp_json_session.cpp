#include "tcp_json_session.h"

TcpJsonSession::TcpJsonSession(asio::ip::tcp::socket socket) : socket_(std::move(socket))
{}

TcpJsonSession::~TcpJsonSession()
{
    Close();
}

std::future<void> TcpJsonSession::Send(const nlohmann::json& j)
{
    auto promise = std::make_shared<std::promise<void>>();
    std::future<void> future = promise->get_future();

    std::string message = j.dump(); // Serialize JSON to string
    std::string header = std::to_string(message.length()) + "\n"; // Preamble with size
    std::string to_send = header + message; // Concatenation of preamble and message

    std::shared_ptr<TcpJsonSession> self = shared_from_this();

    asio::async_write(
        socket_, asio::buffer(to_send),
        [self, promise](std::error_code ec, std::size_t /*length*/)
        {
            if (!ec)
            {
                promise->set_value();
            }
            else
            {
                promise->set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
            }
        });

    return future;
}

std::future<nlohmann::json> TcpJsonSession::Receive()
{
    auto promise = std::make_shared<std::promise<nlohmann::json>>();

    // Asynchronously read up to the newline character (size preamble)
    auto header_buffer = std::make_shared<std::string>();
    asio::async_read_until(
        socket_, asio::dynamic_buffer(*header_buffer), '\n',
        [self = shared_from_this(), promise, header_buffer](std::error_code ec, std::size_t length)
        {
            if (!ec)
            {
                std::size_t message_length = std::stoi(*header_buffer);
                auto message_buffer = std::make_shared<std::vector<char>>(message_length);

                // Asynchronously read the actual JSON message
                asio::async_read(
                    self->socket_, asio::buffer(*message_buffer),
                    [promise, message_buffer](std::error_code ec, std::size_t /*length*/)
                    {
                        if (!ec)
                        {
                            try
                            {
                                auto j = nlohmann::json::parse(message_buffer->begin(), message_buffer->end());
                                promise->set_value(j);
                            }
                            catch (nlohmann::json::parse_error&)
                            {
                                promise->set_exception(std::make_exception_ptr(std::runtime_error("JSON parse error")));
                            }
                        }
                        else
                        {
                            promise->set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
                        }
                    });
            }
            else
            {
                promise->set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
            }
        });

    return promise->get_future();
}

void TcpJsonSession::Close()
{
    if (socket_.is_open())
    {
        std::error_code ec;
        (void)socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
        (void)socket_.close(ec);
    }
}