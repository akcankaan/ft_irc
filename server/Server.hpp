#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <poll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include "../channel/Channel.hpp"
#include "../client/Client.hpp"

#define MAX_CLIENTS 100

class Server {
    private:
        int _port;
        std::string _password;
        int _server_fd;
        std::vector<pollfd> _poll_fds;
        std::map<int, Client *> _clients;
        std::map<std::string, Channel *> _channels;

        void setupSocket();
        void acceptNewClient();
        void handleClientMessage(int fd);
        void removeClient(int fd);
    public:
        Server(int port, const std::string& password);
        ~Server();
        void run();
        void handleJoinCommand(int fd, const std::string &channelName);
};

#endif
