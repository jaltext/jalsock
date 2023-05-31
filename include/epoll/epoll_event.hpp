#pragma once

#include <sys/epoll.h>

#include "types/aliases.hpp"

enum EpollEventFlag : uint32_t {
    None = 0,
    In = EPOLLIN,
    Out = EPOLLOUT,
    Priority = EPOLLPRI,
    Error = EPOLLERR,
    HangUp = EPOLLHUP,
    ReadHangUp = EPOLLRDHUP,
    EdgeTriggered = EPOLLET,
    OneShot = EPOLLONESHOT,
    WakeUp = EPOLLWAKEUP,
    Exclusive = EPOLLEXCLUSIVE,
};

using EpollEventMask = uint32_t;

class EpollEvent {
public:
    EpollEvent() = default;
    EpollEvent(const EpollEvent&) = default;
    EpollEvent(EpollEvent&&) = default;
    EpollEvent& operator=(const EpollEvent&) = default;
    EpollEvent& operator=(EpollEvent&&) = default;

    EpollEvent(FileDesc fd, EpollEventMask events);

    FileDesc fd() const;
    EpollEventMask events() const;
    epoll_event& data();
    const epoll_event& data() const;

    bool contains(EpollEventFlag flag) const;

private:
    epoll_event m_data{};
};