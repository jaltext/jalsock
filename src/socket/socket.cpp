#include "socket/socket.hpp"

#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>

#include "jalsock.hpp"
#include "types/aliases.hpp"
#include "types/sockaddr.hpp"

Socket::Socket(AIFamily domain, AISockType sock_type, AIProtocol protocol)
    : m_fd{::socket(static_cast<int>(domain), static_cast<int>(sock_type),
                    static_cast<int>(protocol))},
      m_type{sock_type} {}

Socket::Socket(Socket&& other) {
    if (&other != this) {
        m_fd = other.m_fd;
        m_type = other.m_type;

        other.m_fd = -1;
    }
}

Socket& Socket::operator=(Socket&& other) {
    if (&other != this) {
        m_fd = other.m_fd;
        m_type = other.m_type;

        other.m_fd = -1;
    }

    return *this;
}

Socket::~Socket() { jalsock::close(m_fd); }

FileDesc Socket::fd() const { return m_fd; }

AISockType Socket::type() const { return m_type; }

bool Socket::error() const { return m_fd == -1; }

bool Socket::setOpt(int level, SockOpt optname) {
    static const int yes = 1;
    int status =
        ::setsockopt(m_fd, level, static_cast<int>(optname), &yes, sizeof(yes));

    return status == 0;
}

std::pair<FileDesc, SockAddr> Socket::accept() {
    SockAddr client_addr{};
    socklen_t size = sizeof(client_addr.data());
    return {
        ::accept(m_fd, reinterpret_cast<sockaddr*>(&client_addr.data()), &size),
        client_addr};
}
