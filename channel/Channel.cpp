#include "Channel.hpp"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>

Channel::Channel(const std::string &name)
    : _name(name), _inviteOnly(false), _userLimit(0), _hasUserLimit(false), topicRestrict(false){}

Channel::~Channel() {
        _clients.clear();
        _invited.clear();
        _operators.clear();
}

const std::string &Channel::getName() const {
    return _name;
}

void Channel::addClient(Client *client) {
    if (!hasClient(client)) {
        _clients.push_back(client);

        if (_clients.size() == 1)
        {

            Client *newOp = getClients()[0];
			addOperator(newOp->getNickname());

			std::string modeMsg = ":@localhost MODE " + _name +
				" +o " + newOp->getNickname() + "\r\n";
			broadcast(modeMsg, NULL);
        }
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
    std::string senderNick = sender ? sender->getNickname() : "";

    for (size_t i = 0; i < _clients.size(); ++i) {
        if (_clients[i] == NULL)
            continue;

        if (!sender || _clients[i]->getNickname() != senderNick) {
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
            _operators.erase(client->getNickname());
            _invited.erase(client->getNickname());
            break;
        }
    }

}

void Channel::kickClient(Client *by, Client *target, const std::string &reason) {
    if (!isOperator(by->getNickname())) {
        return;
    }

    if (!hasClient(target)) {
        return;
    }

    std::string msg = buildKickMsg(by, _name, target, reason);
    broadcast(msg, NULL);
    removeClient(target);

    send(target->getFd(), msg.c_str(), msg.length(), 0);
}

bool Channel::isOperator(std::string nickname) const {
    for (std::set<std::string>::const_iterator it = _operators.begin(); it != _operators.end(); ++it) {
        if ((*it) == nickname) {
            return true;
        }
    }
    return false;
}

const std::string &Channel::getTopic() const { return _topic; }

void Channel::setTopic(const std::string &topic) { _topic = topic; }

void Channel::inviteClient(Client *target) {
    _invited.insert(target->getNickname());
}

bool Channel::isInvited(const Client *client) const {
    return _invited.find(client->getNickname()) != _invited.end();
}

void Channel::setInviteOnly(bool enabled) { _inviteOnly = enabled; }

bool Channel::isInviteOnly() const { return _inviteOnly; }

void Channel::setPassword(const std::string &pass) { _password = pass; }

void Channel::clearPassword() { _password.clear(); }

const std::string &Channel::getPassword() const { return _password; }

bool Channel::hasPassword() const { return !_password.empty(); }

void Channel::setUserLimit(int limit) {
    _userLimit = limit;
    _hasUserLimit = true;
}

void Channel::clearUserLimit() {
    _userLimit = 0;
    _hasUserLimit = false;
}

bool Channel::hasUserLimit() const { return _hasUserLimit; }

bool Channel::isFull() const { return _hasUserLimit && static_cast<int>(_clients.size()) >= _userLimit; }

void Channel::addOperator(std::string nickname) {
    bool inChannel = false;
    for (size_t i = 0; i < _clients.size(); ++i) {
        if (_clients[i]->getNickname() == nickname) {
            inChannel = true;
            break;
        }
    }
    if (!inChannel) {
        return;
    }

    _operators.insert(nickname);
}

void Channel::removeOperator(std::string nickname) { _operators.erase(nickname); }


void Channel::setTopicRestrict(bool value) {
    topicRestrict = value;
}

bool Channel::isTopicRestricted() const {
    return topicRestrict;
}

const std::vector<Client*> &Channel::getClients() const {
    return _clients;
}

bool Channel::isClientInChannel(Client *client) const
{
	for (std::vector<Client*>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (*it == client)
			return true;
	}
	return false;
}

size_t Channel::operatorCount() const {
    return _operators.size();
}
