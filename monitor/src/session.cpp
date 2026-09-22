#include "session.h"

#include <iostream>

Session::Session(asio::ip::tcp::socket socket, int id)
    : _socket(std::move(socket))
    , _id(id)
{
    AsyncRead();
}

void Session::AsyncRead()
{
    asio::async_read_until(_socket, _recv_buffer, '\n', 
        std::bind(&Session::HandleRead, this, std::placeholders::_1, std::placeholders::_2));
}

void Session::HandleRead(asio::error_code ec, std::size_t length)
{
    if (!ec) {
        std::string data{
            std::istreambuf_iterator<char>(&_recv_buffer),
            std::istreambuf_iterator<char>()
        };

        std::cout << "received: " << data << std::endl;
        AsyncRead();

    } else {
        if (ec != asio::error::eof) {
            std::cerr << "Error when receive data: " << ec.message() << std::endl;
        } // otherwise, handle somehow closing connection
    }
}
