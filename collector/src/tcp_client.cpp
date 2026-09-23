#include "tcp_client.h"

#include <iostream>
#include <stdexcept>


TCPClient::TCPClient()
    : _socket(_io_context)
    , _resolver(_io_context)
{
}

TCPClient::~TCPClient()
{
    asio::error_code ec;
    _socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
    if (ec) {
        std::cerr << "Error when shutdownd socket: " << ec.message() << std::endl;
    }

    _socket.close(ec);
    if (ec) {
        std::cerr << "Error when close socket: " << ec.message() << std::endl;
    }
}

bool TCPClient::Connect(const std::string& host, unsigned short port)
{
    asio::error_code ec;
    auto endpoints = _resolver.resolve(host, std::to_string(port), ec);
    if (ec) {
        std::cerr << "Error when resolving endpoints: " << ec.message() << std::endl;
        return false;
    }

    asio::connect(_socket, endpoints, ec);
    if (ec) {
        std::cerr << "Error when connect to host: " << ec.message() << std::endl;
        return false;
    }

    return true;
}

bool TCPClient::Send(const std::string& data)
{
    asio::error_code ec;
    asio::write(_socket, asio::buffer(data), ec);
    if (ec) {
        std::cerr << "Error when send data: " << ec.message() << std::endl;
        return false;
    }
    return true;
}
