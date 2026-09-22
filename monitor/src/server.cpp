#include "server.h"

#include <iostream>

Server::Server(unsigned short port)
    : _acceptor(_io_context, 
        asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
{
    AsyncAccept();
}

void Server::Run()
{
    _io_context.run();
}

void Server::AsyncAccept()
{
    _acceptor.async_accept(std::bind(&Server::HandleAccept, this, 
        std::placeholders::_1, std::placeholders::_2));
}

void Server::HandleAccept(asio::error_code ec, asio::ip::tcp::socket socket)
{
    if (!ec) {
        std::cout << "Accepted connection from "
            << socket.remote_endpoint().address().to_string() << ':'
            << socket.remote_endpoint().port() << std::endl;
        
        const int session_id = _next_session_id;
        try {
            std::unique_ptr<Session> session = 
                std::make_unique<Session>(std::move(socket), session_id);
            _sessions[session_id] = std::move(session);
            _next_session_id++;
        } catch (const std::exception& ex) {
            std::cerr << "Exception in Server::HandleAccept - " << ex.what() << std::endl;
        }
    } else {
        std::cerr << "Error when accept connection: " << ec.message() << std::endl;
    }
    AsyncAccept();
}
