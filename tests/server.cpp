#include "server.hpp"

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string_view>

#include "epoll/epoll_event.hpp"
#include "jalsock.hpp"
#include "socket/socket.hpp"

Server::Server(std::string_view port) : m_port{port} {}

Server::~Server() {}

void Server::run() {
    init();

    std::cerr << "Server is running on port " << m_port << std::endl;

    while (true) {
        m_epoll.wait(-1);
        std::cerr << "Event count: " << m_epoll.eventCount() << std::endl;

        for (auto& event : m_epoll.events()) {
            if (!event.contains(EpollEventFlag::In)) {
                continue;
            }

            if (event.fd() == m_listen_sock.fd()) {
                handleNewConnection();
            } else {
                handleRequest(event);
            }
        }
    }
}

void Server::init() {
    AddrInfo hints;
    hints.setFlag(AIFlag::Passive)
        .setFamily(AIFamily::Unspec)
        .setSocket(AISockType::Stream);

    const auto& [err, addresses] = jalsock::getAddressInfo({}, m_port, hints);

    if (err != ErrAI::Success) {
        std::cerr << "Can't get address info: "
                  << gai_strerror(static_cast<int>(err)) << std::endl;
        throw std::runtime_error{gai_strerror(static_cast<int>(err))};
    }

    for (const auto& address : addresses) {
        Socket socket(address.family(), address.sockType(), address.protocol());

        if (socket.error()) {
            std::cerr << "Can't create socket: " << std::strerror(errno)
                      << std::endl;
            continue;
        }

        if (!socket.setOpt(SOL_SOCKET, SockOpt::ReuseAddr)) {
            std::cerr << "Can't set socket options: " << std::strerror(errno)
                      << std::endl;
            continue;
        }

        if (!jalsock::bind(socket, address)) {
            std::cerr << "Can't bind socket: " << std::strerror(errno)
                      << std::endl;
            continue;
        }

        m_listen_sock = std::move(socket);
        break;
    }

    if (m_listen_sock.error()) {
        throw std::runtime_error{"Can't bind socket"};
    }

    if (!jalsock::listen(m_listen_sock, backlog)) {
        throw std::runtime_error{"Can't listen socket"};
    }

    m_epoll.add(m_listen_sock.fd(), EpollEventFlag::In);
}

void Server::handleNewConnection() {
    const auto& [client_fd, client_addr] = m_listen_sock.accept();

    if (client_fd == -1) {
        std::cerr << "Can't accept connection: " << std::strerror(errno)
                  << std::endl;
        return;
    }

    m_epoll.add(client_fd, EpollEventFlag::In | EpollEventFlag::EdgeTriggered);

    std::cerr << "Connection from "
              << jalsock::networkToPresentation(client_addr) << " (socket "
              << client_fd << ")" << std::endl;
}

void Server::handleRequest(const EpollEvent& event) {
    const auto client_fd = event.fd();
    const auto& [len, message] = jalsock::recv(client_fd, 0);

    if (len <= 0) {
        if (len == 0) {
            std::cerr << "Disconnected from socket " << client_fd << std::endl;
        } else {
            std::cerr << "Can't receive data: " << std::strerror(errno)
                      << std::endl;
        }

        m_epoll.remove(client_fd);
        jalsock::close(client_fd);
        return;
    }

    std::cerr << "Socket " << client_fd << ", message: " << message
              << std::endl;

    if (message == "/quit") {
        m_epoll.remove(client_fd);
        jalsock::close(client_fd);
        return;
    }

    for (const auto& event : m_epoll.events()) {
        if (event.fd() == m_listen_sock.fd() || event.fd() == client_fd) {
            continue;
        }

        std::cerr << "Send message to socket " << event.fd() << std::endl;
        jalsock::send(event.fd(), message, client_fd);
    }
}