#pragma once

#include <asio.hpp>


class Session final
{
    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;

public:
    Session(asio::ip::tcp::socket socket, int id);
    ~Session() = default;

private:
    void AsyncRead();
    void HandleRead(asio::error_code ec, std::size_t length);
    
private:
    asio::ip::tcp::socket _socket;
    int _id{-1};
    asio::streambuf _recv_buffer;
};