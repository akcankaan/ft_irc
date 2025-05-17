#include "Server.hpp"
#include <iostream>
#include <stdexcept>     // std::runtime_error
#include <cstring>       // memset
#include <cstdlib>       // atoi, etc.
#include <unistd.h>      // close
#include <fcntl.h>       // fcntl
#include <poll.h>        // poll
#include <sys/socket.h>  // socket, bind, listen, accept
#include <netinet/in.h>  // sockaddr_in
#include <arpa/inet.h>   // inet_ntoa, etc.

Server::Server(int port, const std::string &password)
    : _port(port), _password(password) {
    std::cout << "Server initialized with port: " << _port
              << " and password: " << _password << std::endl;
}

Server::~Server() {
    std::cout << "Server shutting down." << std::endl;
}

Channel* Server::getOrCreateChannel(const std::string &name) {
    if (_channels.find(name) == _channels.end()) {
        _channels[name] = new Channel(name);
        std::cout << "Channel created: " << name << std::endl;
    }
    return _channels[name];
}

void Server::run() {
    _serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocket < 0)
        throw std::runtime_error("Socket creation failed");

    int opt = 1;
    if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("setsockopt failed");

    if (fcntl(_serverSocket, F_SETFL, O_NONBLOCK) < 0)
        throw std::runtime_error("fcntl failed");

    sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(_port);

    if (bind(_serverSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        throw std::runtime_error("Bind failed");

    if (listen(_serverSocket, 10) < 0)
        throw std::runtime_error("Listen failed");

    std::cout << "Server listening on port " << _port << std::endl;

    _pollFds.clear();
    struct pollfd serverPollFd = { _serverSocket, POLLIN, 0 };
    _pollFds.push_back(serverPollFd);

    while (true) {
        int pollResult = poll(&_pollFds[0], _pollFds.size(), -1);
        if (pollResult < 0)
            throw std::runtime_error("Poll failed");

        for (size_t i = 0; i < _pollFds.size(); ++i) {
            int fd = _pollFds[i].fd;

            if (_pollFds[i].revents & POLLIN) {
                // Yeni bağlantı
                if (fd == _serverSocket) {
                    sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int client_fd = accept(_serverSocket, (sockaddr *)&client_addr, &client_len);
                    if (client_fd >= 0)
                        addClient(client_fd);
                }
                // Veri geldi
                else {
                    char buffer[512];
                    std::memset(buffer, 0, sizeof(buffer));
                    ssize_t bytes = recv(fd, buffer, sizeof(buffer), 0);

                    if (bytes <= 0) {
                        removeClient(fd);
                    } else {
                        Client *client = _clients[fd];
                        client->appendBuffer(std::string(buffer, bytes));

                        std::string &full = const_cast<std::string&>(client->getBuffer());
                        size_t pos;
                        while ((pos = full.find("\n")) != std::string::npos) {
                            std::string line = full.substr(0, pos);
                            if (!line.empty() && line[line.size() - 1] == '\r')
                                line.erase(line.size() - 1);
                            CommandHandler::handleCommand(client, line);
                            full.erase(0, pos + 1);
                        }
                    }
                }
            }
        }
    }

    close(_serverSocket);
}

void Server::addClient(int client_fd) {
    // Socket'i non-blocking moda al
    fcntl(client_fd, F_SETFL, O_NONBLOCK);

    // Yeni client oluştur
    Client* newClient = new Client(client_fd);
    newClient->setServer(this);  // Server referansı ver
    _clients[client_fd] = newClient;

    // pollfd yapılandırması (DİKKAT: revents sıfırlanmalı!)
    struct pollfd pfd;
    pfd.fd = client_fd;
    pfd.events = POLLIN;
    pfd.revents = 0;  // ✅ uninitialized uyarılarını engeller
    _pollFds.push_back(pfd);

    std::cout << "Client added: " << client_fd << std::endl;
}


void Server::removeClient(int client_fd) {
    std::cout << "Client removed: " << client_fd << std::endl;

    close(client_fd);
    delete _clients[client_fd];
    _clients.erase(client_fd);

    for (std::vector<struct pollfd>::iterator it = _pollFds.begin(); it != _pollFds.end(); ++it) {
        if (it->fd == client_fd) {
            _pollFds.erase(it);
            break;
        }
    }
}

std::map<std::string, Channel*> &Server::getChannelMap() {
    return _channels;
}

const std::map<int, Client*> &Server::getClientMap() const {
    return _clients;
}
