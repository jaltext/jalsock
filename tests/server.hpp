#pragma once

#include <string_view>

#include "types/aliases.hpp"
#include "socket/socket.hpp"
#include "epoll/epoll.hpp"

class Server {
public:
    Server() = delete;
    Server(std::string_view port);
    ~Server();

    void run();

private:
    static constexpr int backlog = 10;

    std::string_view m_port;
    Socket m_listen_sock{};
    EpollInstance m_epoll{};

    void init();
    void handleNewConnection();
    void handleRequest(const EpollEvent& event);
};