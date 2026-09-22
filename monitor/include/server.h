#pragma once

#include <asio.hpp>
#include <memory>
#include <unordered_map>

#include "session.h"

class Server final
{
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

public:
    explicit Server(unsigned short port);
    ~Server() = default;

    void Run();

private:
    void AsyncAccept();
    void HandleAccept(asio::error_code ec, asio::ip::tcp::socket socket);

private:
    asio::io_context _io_context;
    asio::ip::tcp::acceptor _acceptor;
    std::unordered_map<int, std::unique_ptr<Session>> _sessions;
    int _next_session_id{0};
};