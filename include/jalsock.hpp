#pragma once

#include <cstddef>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#include "socket/socket.hpp"
#include "types/addr_info.hpp"
#include "types/sockaddr.hpp"
#include "types/aliases.hpp"

namespace jalsock {

bool bind(const Socket& socket, const AddrInfo& addr);

void close(FileDesc fd);

std::optional<std::ptrdiff_t> send(FileDesc fd, const std::string_view view,
                                   int flags);

std::pair<int, std::string> recv(FileDesc fd, int flags);

bool listen(const Socket& socket, int backlog);

std::pair<ErrAI, std::vector<AddrInfo>> getAddressInfo(
    const std::string_view node, const std::string_view service,
    const AddrInfo& hints);

std::string_view networkToPresentation(const SockAddr& address);

}  // namespace jalsock