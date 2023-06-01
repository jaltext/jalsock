#include "jalsock.hpp"

#include <arpa/inet.h>
#include <unistd.h>

#include <array>
#include <cassert>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <vector>

#include "types/addr_info.hpp"
#include "types/aliases.hpp"

namespace jalsock {

bool bind(const Socket& socket, const AddrInfo& addr) {
    return ::bind(socket.fd(), addr.address(), addr.addressLen()) == 0;
}

void close(FileDesc fd) {
    if (fd == -1) {
        return;
    }

    if (::close(fd) == -1) {
        throw std::runtime_error("Failed to close socket");
    }
}

std::optional<std::ptrdiff_t> send(FileDesc fd, const std::string_view view,
                                   int flags) {
    std::ptrdiff_t len = ::send(fd, view.data(), view.size(), flags);

    if (len == -1) {
        return std::nullopt;
    }

    return len;
}

std::optional<std::string> recv(FileDesc fd, int flags) {
    static std::array<char, 1 << 16> buffer;
    int len = ::recv(fd, buffer.data(), buffer.size(), flags);

    if (len == -1) {
        return std::nullopt;
    }

    return std::string(buffer.data(), len);
}

bool listen(const Socket& socket, int backlog) {
    assert(socket.type() == AISockType::Stream ||
           socket.type() == AISockType::SeqPacket);

    return ::listen(socket.fd(), backlog) == 0;
}

bool connect(const Socket& socket, const AddrInfo& address) {
    return ::connect(socket.fd(), address.address(), address.addressLen()) == 0;
}

std::pair<ErrAI, std::vector<AddrInfo>> getAddressInfo(
    const std::string_view node, const std::string_view service,
    const AddrInfo& hints) {
    addrinfo* result;
    const char* node_ptr = node.empty() ? nullptr : node.data();

    ErrAI error = static_cast<ErrAI>(
        ::getaddrinfo(node_ptr, service.data(), &hints.data(), &result));

    std::vector<AddrInfo> addresses;

    for (addrinfo* addr = result; addr != nullptr; addr = addr->ai_next) {
        addresses.push_back(*addr);
    }
    freeaddrinfo(result);

    return {error, addresses};
}

void* getInAddr(const SockAddr& address) {
    if (address.family() == AIFamily::IPv4) {
        return &(((sockaddr_in*)&address.data())->sin_addr);
    } else {
        return &(((sockaddr_in6*)&address.data())->sin6_addr);
    }
}

std::string_view networkToPresentation(const SockAddr& address) {
    static char buffer[INET6_ADDRSTRLEN];
    const char* result =
        ::inet_ntop(static_cast<int>(address.family()), getInAddr(address),
                    buffer, sizeof(buffer));

    return result == nullptr ? "" : result;
}

}  // namespace jalsock
