#pragma once

#include <asio.hpp>

#include <string>

class TCPClient final
{
public:
    TCPClient();
    ~TCPClient();

    bool Connect(const std::string& hostname, unsigned short port);
    bool Send(const std::string& data);

private:
    asio::io_context _io_context;
    asio::ip::tcp::socket _socket;
    asio::ip::tcp::resolver _resolver;
};
