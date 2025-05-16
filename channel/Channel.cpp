#include "Channel.hpp"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>

Channel::Channel(const std::string &name)
    : _name(name) {}

Channel::~Channel() {}

const std::string &Channel::getName() const {
    return _name;
}

void Channel::addClient(Client *client) {
    if (!hasClient(client))
        _clients.push_back(client);
}

bool Channel::hasClient(Client *client) const {
    for (size_t i = 0; i < _clients.size(); ++i) {
        if (_clients[i] == client)
            return true;
    }
    return false;
}

void Channel::broadcast(const std::string &message, Client *sender) {
    for (size_t i = 0; i < _clients.size(); ++i) {
        if (_clients[i] != sender) {
            send(_clients[i]->getFd(), message.c_str(), message.length(), 0);
        }
    }
}
