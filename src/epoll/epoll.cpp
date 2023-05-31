#include "epoll/epoll.hpp"

#include <sys/epoll.h>

#include <algorithm>
#include <span>
#include <stdexcept>

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
    if (::epoll_ctl(m_fd, EPOLL_CTL_ADD, fd, &event.data())) {
        throw std::runtime_error{"Failed to add fd to epoll instance"};
    }

    m_fds.push_back(fd);
}

void EpollInstance::modify(FileDesc fd, EpollEventMask events) {
    EpollEvent event{fd, events};
    if (::epoll_ctl(m_fd, EPOLL_CTL_MOD, fd, &event.data())) {
        throw std::runtime_error{"Failed to modify fd in epoll instance"};
    }
}

void EpollInstance::remove(FileDesc fd) {
    if (::epoll_ctl(m_fd, EPOLL_CTL_DEL, fd, nullptr)) {
        throw std::runtime_error{"Failed to remove fd from epoll instance"};
    }

    m_fds.erase(std::remove(m_fds.begin(), m_fds.end(), fd), m_fds.end());
}

void EpollInstance::wait(int timeout) {
    m_event_count =
        ::epoll_wait(m_fd, reinterpret_cast<epoll_event*>(m_events.data()),
                     m_events.size(), timeout);

    if (m_event_count < 0) {
        throw std::runtime_error{"Failed to wait on epoll instance"};
    }
}

FileDesc EpollInstance::fd() const { return m_fd; }

int EpollInstance::eventCount() const { return m_event_count; }

std::span<EpollEvent> EpollInstance::events() {
    return {m_events.data(), m_event_count};
}

std::span<FileDesc> EpollInstance::fds() {
    return {m_fds.data(), m_fds.size()};
}
