#include "Client.hpp"
#include <string>
#include <algorithm>

Client::Client(int fd) : _fd(fd), _isAuthenticated(false), _hasGivenPassword(false), _shouldDisconnect(false) {}

Client::~Client() {
    _joinedChannels.clear();
    _buffer.clear();
    _nickname.clear();
    _username.clear();
    _realname.clear();
    _hostname.clear();
}


int Client::getFd() const { return _fd; }

const std::string &Client::getNickname() const { return _nickname; }

const std::string &Client::getUsername() const { return _username; }

const std::string &Client::getBuffer() const { return _buffer; }

bool Client::isAuthenticated() const { return _isAuthenticated; }

bool Client::hasGivenPassword() const { return _hasGivenPassword; }

void Client::markPasswordGiven() { _hasGivenPassword = true; }

void Client::setNickname(const std::string &nick) { _nickname = nick; }

void Client::setUsername(const std::string &user) { _username = user; }

void Client::appendBuffer(const std::string &data) { _buffer += data; }

void Client::clearBuffer() { _buffer.clear(); }

void Client::authenticate() { _isAuthenticated = true; }

void Client::setServer(Server *server) { _server = server; }

Server *Client::getServer() const { return _server; }

void Client::removeChannel(const std::string& channelName) {
	std::vector<std::string>::iterator it = std::find(_joinedChannels.begin(), _joinedChannels.end(), channelName);
	if (it != _joinedChannels.end())
		_joinedChannels.erase(it);
}

void Client::setDisconnected(bool value)
{
	_shouldDisconnect = value;
}
bool Client::shouldDisconnect() const
{
	return _shouldDisconnect;
}

const std::vector<std::string>& Client::getJoinedChannels() const {
    return _joinedChannels;
}

