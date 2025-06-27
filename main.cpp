#include "server/Server.hpp"
#include <csignal>
#include <iostream>
#include <cstdlib>

static Server* g_server = NULL;

void handle_sigint(int) {
    delete g_server;
    std::exit(0);
}

int main(int argc, char **argv) {
    std::signal(SIGINT, handle_sigint);

    if (argc != 3) {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }

    int port = std::atoi(argv[1]);
    std::string password = argv[2];

    try {
        g_server = new Server(port, password);
        g_server->run();
        delete g_server;
    } catch (const std::exception &e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        delete g_server;
        return 1;
    }

    return 0;
}
