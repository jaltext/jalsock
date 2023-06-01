#include "client.hpp"

#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string_view>

#include "jalsock.hpp"
#include "types/addr_info.hpp"

Client::Client(std::string_view host, std::string_view port)
    : m_host{host}, m_port{port} {}

void Client::run() {
    init();

    std::cerr << "Connected to " << m_host << ":" << m_port << std::endl;

    while (true) {
        m_epoll.wait(0);

        for (const auto& event : m_epoll.events()) {
            if (!event.contains(EpollEventFlag::In)) {
                continue;
            }

            if (event.fd() == m_server_sock.fd()) {
                handleServerInput();
            } else if (event.fd() == STDIN_FILENO) {
                handleStdinInput();
            }
        }
    }
}

void Client::init() {
    AddrInfo hints;
    hints.setFamily(AIFamily::Unspec).setSocket(AISockType::Stream);

    const auto& [error, addresses] =
        jalsock::getAddressInfo(m_host, m_port, hints);

    if (error != ErrAI::Success) {
        std::cerr << "Can't get address info: "
                  << gai_strerror(static_cast<int>(error)) << std::endl;

        throw std::runtime_error("Failed to get address info");
    }

    for (const auto& address : addresses) {
        Socket socket(address.family(), address.sockType(), address.protocol());

        if (socket.error()) {
            std::cerr << "Can't create socket: " << std::strerror(errno)
                      << std::endl;
            continue;
        }

        if (!jalsock::connect(socket, address)) {
            std::cerr << "Can't connect socket: " << std::strerror(errno)
                      << std::endl;
            continue;
        }

        m_server_sock = std::move(socket);
        break;
    }

    if (m_server_sock.error()) {
        throw std::runtime_error{"Can't connect socket"};
    }

    m_epoll.add(m_server_sock.fd(), EpollEventFlag::In);
    m_epoll.add(STDIN_FILENO, EpollEventFlag::In);
}

void Client::handleServerInput() {
    const auto& recv_opt = jalsock::recv(m_server_sock.fd(), 0);

    if (!recv_opt) {
        std::cerr << "Can't receive data: " << std::strerror(errno)
                  << std::endl;
        return;
    }

    const auto& message = *recv_opt;

    if (message.empty()) {
        std::cerr << "Server closed connection" << std::endl;
        return;
    }

    std::cout << message.data() << std::endl;
}

void Client::handleStdinInput() {
    std::string message;
    std::getline(std::cin, message);

    if (message.empty()) {
        return;
    }

    if (!jalsock::send(m_server_sock.fd(), message, 0)) {
        std::cerr << "Can't send data: " << std::strerror(errno) << std::endl;
        return;
    }

    if (message == "/quit") {
        std::cerr << "Client closed connection" << std::endl;
        std::exit(0);
    }
}