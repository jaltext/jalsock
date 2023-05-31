add_library(${PROJECT_NAME} STATIC
    epoll/epoll.cpp
    epoll/epoll_event.cpp

    socket/socket.cpp

    types/addr_info.cpp
    types/sockaddr.cpp

    jalsock.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/include)

