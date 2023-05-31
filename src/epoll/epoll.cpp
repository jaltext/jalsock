#include "epoll/epoll.hpp"

#include <sys/epoll.h>

#include <span>

#include "epoll/epoll_event.hpp"
#include "jalsock.hpp"

EpollInstance::EpollInstance() : m_fd{::epoll_create1(EPOLL_CLOEXEC)} {}

EpollInstance::EpollInstance(EpollInstance&& other) {
    if (this != &other) {
        m_fd = other.m_fd;
        m_events = std::move(other.m_events);
        m_event_count = other.m_event_count;

        other.m_fd = -1;
        other.m_event_count = 0;
    }
}

EpollInstance& EpollInstance::operator=(EpollInstance&& other) {
    if (this != &other) {
        m_fd = other.m_fd;
        m_events = std::move(other.m_events);
        m_event_count = other.m_event_count;

        other.m_fd = -1;
        other.m_event_count = 0;
    }
    return *this;
}

EpollInstance::~EpollInstance() { jalsock::close(m_fd); }

void EpollInstance::add(FileDesc fd, EpollEventMask events) {
    EpollEvent event{fd, events};
    ::epoll_ctl(m_fd, EPOLL_CTL_ADD, fd, &event.data());
}

void EpollInstance::modify(FileDesc fd, EpollEventMask events) {
    EpollEvent event{fd, events};
    ::epoll_ctl(m_fd, EPOLL_CTL_MOD, fd, &event.data());
}

void EpollInstance::remove(FileDesc fd) {
    ::epoll_ctl(m_fd, EPOLL_CTL_DEL, fd, nullptr);
}

void EpollInstance::wait(int timeout) {
    m_event_count =
        ::epoll_wait(m_fd, reinterpret_cast<epoll_event*>(m_events.data()),
                     m_events.size(), timeout);
}

FileDesc EpollInstance::fd() const { return m_fd; }

int EpollInstance::eventCount() const { return m_event_count; }

std::span<EpollEvent> EpollInstance::events() {
    return {m_events.data(), m_event_count};
}