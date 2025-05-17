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
    if (!hasClient(client)) {
        _clients.push_back(client);

        // Eğer bu kanalda ilk kullanıcıysa, operator yap
        if (_clients.size() == 1)
            setOperator(client);
    }
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

static std::string buildKickMsg(Client *from, const std::string &channel, Client *target, const std::string &reason) {
    return ":" + from->getNickname() + " KICK " + channel + " " + target->getNickname() + " :" + reason + "\r\n";
}

void Channel::removeClient(Client *client) {
    for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (*it == client) {
            _clients.erase(it);
            break;
        }
    }
}

void Channel::kickClient(Client *by, Client *target, const std::string &reason) {
    if (!isOperator(by)) {
        send(by->getFd(), "You are not channel operator\r\n", 31, 0);
        return;
    }

    if (!hasClient(target)) {
        send(by->getFd(), "Target not in channel\r\n", 24, 0);
        return;
    }

    std::string msg = buildKickMsg(by, _name, target, reason);
    broadcast(msg, NULL);
    removeClient(target);

    send(target->getFd(), msg.c_str(), msg.length(), 0);
}

bool Channel::isOperator(Client *client) const {
    return !_clients.empty() && _clients[0] == client;
}

void Channel::setOperator(Client *client) {
    if (_clients.empty())
        _clients.push_back(client);
}

const std::string &Channel::getTopic() const { return _topic; }

void Channel::setTopic(const std::string &topic) { _topic = topic; }

void Channel::inviteClient(Client *target) {
    _invited.insert(target->getNickname());
}

bool Channel::isInvited(const Client *client) const {
    return _invited.find(client->getNickname()) != _invited.end();
}