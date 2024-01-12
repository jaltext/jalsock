#include "server.hpp"

#include <cstring>
#include <iostream>
#include <fstream>
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
        m_epoll.wait(0);

        for (const auto& event : m_epoll.events()) {
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

    m_epoll.add(client_fd, EpollEventFlag::In);

    std::cerr << "Connection from "
              << jalsock::networkToPresentation(client_addr) << " (socket "
              << client_fd << ")" << std::endl;
}

void Server::handleRequest(const EpollEvent& event) {
    const auto client_fd = event.fd();
    const auto& recv_opt = jalsock::recv(client_fd, 0);

    if (!recv_opt) {
        std::cerr << "Can't receive data: " << std::strerror(errno)
                  << std::endl;
        m_epoll.remove(client_fd);
        jalsock::close(client_fd);
        return;
    }

    const auto& message = *recv_opt;

    if (message.empty()) {
        std::cerr << "Disconnected from socket " << client_fd << std::endl;
        m_epoll.remove(client_fd);
        jalsock::close(client_fd);
        return;
    }

    std::cerr << "Socket " << client_fd << ", message: " << message
              << std::endl;

    // `message` is the path to the file to be sent
    // send the file to the client
    // it is either a text file or an mp3 file
    std::string path = std::string(".") + message;
    std::cerr << "Path: " << path << std::endl;

    std::ifstream file(path, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Can't open file: " << std::strerror(errno) << std::endl;
        std::size_t file_size = 0;
        jalsock::send(client_fd, std::string((char*)&file_size, sizeof(file_size)), 0);
        return;
    }

    std::string file_contents((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());

    std::size_t file_size = file_contents.size();
    jalsock::send(client_fd, std::string((char*)&file_size, sizeof(file_size)), 0);

    std::cerr << "File size: " << file_size << std::endl;

    std::size_t total_sent = 0;
    while (total_sent < file_size) {
        std::cerr << "Sending data..." << std::endl;
        std::string remaining_data = file_contents.substr(total_sent);

        std::size_t sent = jalsock::send(client_fd, remaining_data, 0).value_or(-1);

        if (sent == -1) {
            std::cerr << "Can't send data: " << std::strerror(errno)
                      << std::endl;
            break;
        }

        total_sent += sent;

        std::cerr << "Sent " << total_sent << " bytes" << std::endl;
    }
}
