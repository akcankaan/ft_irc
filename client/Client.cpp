#include "Client.hpp"

Client::Client(int fd) : _fd(fd), _authenticated(false) {}

int Client::getFd() const { return _fd; }

const std::string& Client::getNickname() const { return _nickname; }
const std::string& Client::getUsername() const { return _username; }
const std::string& Client::getRealname() const { return _realname; }

bool Client::isAuthenticated() const { return _authenticated; }
std::string& Client::getBuffer() { return _buffer; }

void Client::setNickname(const std::string& nick) { _nickname = nick; }
void Client::setUsername(const std::string& user) { _username = user; }
void Client::setRealname(const std::string& real) { _realname = real; }
void Client::setAuthenticated(bool status) { _authenticated = status; }