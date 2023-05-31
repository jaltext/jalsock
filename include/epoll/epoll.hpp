#pragma once

#include <sys/epoll.h>

#include <array>
#include <cstddef>
#include <span>
#include <vector>

#include "epoll/epoll_event.hpp"
#include "types/aliases.hpp"

enum class EpollCtlOp : int {
    Add = EPOLL_CTL_ADD,
    Modify = EPOLL_CTL_MOD,
    Remove = EPOLL_CTL_DEL,
};

class EpollInstance {
public:
    EpollInstance();

    EpollInstance(const EpollInstance&) = delete;
    EpollInstance& operator=(const EpollInstance&) = delete;

    EpollInstance(EpollInstance&& other);
    EpollInstance& operator=(EpollInstance&& other);

    ~EpollInstance();

    void add(FileDesc fd, EpollEventMask events);
    void modify(FileDesc fd, EpollEventMask events);
    void remove(FileDesc fd);
    void wait(int timeout);

    FileDesc fd() const;
    int eventCount() const;
    std::span<EpollEvent> events();
    std::span<FileDesc> fds();

private:
    static constexpr int max_events = 1024;

    FileDesc m_fd{-1};
    std::array<EpollEvent, max_events> m_events{};
    std::size_t m_event_count{};
    std::vector<FileDesc> m_fds{};
};