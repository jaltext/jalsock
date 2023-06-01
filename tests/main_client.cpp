#include <stdexcept>
#include <iostream>

#include "client.hpp"

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: ./jalsock_client <ip> <port>" << std::endl;
        return 1;
    }

    try {
        Client client{argv[1], argv[2]};
        client.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}