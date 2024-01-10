add_library(jalsock STATIC
    epoll/epoll.cpp
    epoll/epoll_event.cpp

    socket/socket.cpp

    types/addr_info.cpp
    types/sockaddr.cpp

    jalsock.cpp
)

target_include_directories(jalsock PUBLIC
    ${PROJECT_SOURCE_DIR}/include)

