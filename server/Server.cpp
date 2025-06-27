#include "Server.hpp"
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

Server::Server(int port, const std::string &password)
    : _port(port), _password(password) {
    std::cout << "Server initialized with port: " << _port
              << " and password: " << _password << std::endl;
}

Server::~Server() {
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        close(it->first);
        delete it->second;
    }
    _clients.clear();

    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
        delete it->second;
    }
    _channels.clear();

    _pollFds.clear();
    std::vector<struct pollfd>().swap(_pollFds);
    std::cout << "\nServer shutting down and memory cleaned." << std::endl;
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

    for (size_t i = 0; i < _pollFds.size(); ) {
        int fd = _pollFds[i].fd;

        if (_pollFds[i].revents & POLLIN) {
            if (fd == _serverSocket) {
                sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(_serverSocket, (sockaddr *)&client_addr, &client_len);
                if (client_fd >= 0)
                    addClient(client_fd);
                ++i;
            } else {
                char buffer[512];
                std::memset(buffer, 0, sizeof(buffer));
                ssize_t bytes = recv(fd, buffer, sizeof(buffer), 0);

                if (bytes <= 0) {
                    removeClient(fd);
                    continue;
                }

                std::map<int, Client*>::iterator clientIt = _clients.find(fd);
                if (clientIt == _clients.end()) {
                    continue;
                }
                Client *client = clientIt->second;
                client->appendBuffer(std::string(buffer, bytes));

                size_t pos;
                std::string &full = const_cast<std::string&>(client->getBuffer());
                bool clientRemoved = false;


                while ((pos = full.find("\n")) != std::string::npos) {
                    std::string line = full.substr(0, pos);
                    if (!line.empty() && line[line.size() - 1] == '\r')
                        line.erase(line.size() - 1);
                    CommandHandler::handleCommand(client, line);


                    if (_clients.find(fd) == _clients.end()) {
                        clientRemoved = true;
                        break;
                    }

                    if (client->shouldDisconnect()) {
                        full.erase(0, pos + 1);
                        removeClient(fd);
                        clientRemoved = true;
                        break;
                    }
                    full.erase(0, pos + 1);
                }

                if (clientRemoved) {
                    continue;
                }
                ++i;
            }
        } else {
            ++i;
        }
    }
}
close(_serverSocket);
}

void Server::addClient(int client_fd) {
    fcntl(client_fd, F_SETFL, O_NONBLOCK);


    Client* newClient = new Client(client_fd);
    newClient->setServer(this);
    _clients[client_fd] = newClient;
    struct pollfd pfd;
    pfd.fd = client_fd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    _pollFds.push_back(pfd);

    std::cout << "Client added: " << client_fd << std::endl;
}


void Server::removeClient(int client_fd) {
    std::cout << "Client removed: " << client_fd << std::endl;

    if (_clients.count(client_fd)) {
        close(client_fd);
        delete _clients[client_fd];
        _clients.erase(client_fd);
    }

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

std::string Server::getPassword() const {
    return _password;
}

bool Server::isNicknameInUse(const std::string &nickname) const {
    for (std::map<int, Client*>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second->getNickname() == nickname)
            return true;
    }
    return false;
}
