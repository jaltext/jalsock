#pragma once

#include "types/aliases.hpp"
#include "types/sockaddr.hpp"

#include <utility>

class Socket {
public:
    Socket() = default;
    Socket(AIFamily domain, AISockType sock_type, AIProtocol protocol);

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other);
    Socket& operator=(Socket&& other);

    ~Socket();

    FileDesc fd() const;
    AISockType type() const;
    bool error() const;
    bool setOpt(int level, SockOpt optname);
    std::pair<FileDesc, SockAddr> accept();

private:
    FileDesc m_fd{-1};
    AISockType m_type;
};
