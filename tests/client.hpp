#pragma once

#include <string_view>

#include "socket/socket.hpp"
#include "epoll/epoll.hpp"

class Client {
public:
    Client(std::string_view host, std::string_view port);

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    Client(Client&&) = delete;
    Client& operator=(Client&&) = delete;

    void run();

private:
    std::string_view m_host;
    std::string_view m_port;
    Socket m_server_sock{};
    EpollInstance m_epoll{};

    void init();
    void handleServerInput();
    void handleStdinInput();
};