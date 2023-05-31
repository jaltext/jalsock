#include "epoll/epoll_event.hpp"

#include "jalsock.hpp"

EpollEvent::EpollEvent(FileDesc fd, EpollEventMask events)
    : m_data{.events = events, .data = {.fd = fd}} {}

FileDesc EpollEvent::fd() const { return m_data.data.fd; }

EpollEventMask EpollEvent::events() const { return m_data.events; }

epoll_event& EpollEvent::data() { return m_data; }

const epoll_event& EpollEvent::data() const { return m_data; }

bool EpollEvent::contains(EpollEventFlag flag) const {
    return m_data.events & flag;
}