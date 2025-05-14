#include "Channel.hpp"

Channel::Channel(const std::string& name) : _name(name), _limit(-1), _inviteOnly(false), _topicLock(false) {}

const std::string& Channel::getName() const { return _name; }
const std::string& Channel::getTopic() const { return _topic; }
void Channel::setTopic(const std::string& topic) { _topic = topic; }

void Channel::addMember(int fd) { _members.insert(fd); }
void Channel::removeMember(int fd) {
    _members.erase(fd);
    _operators.erase(fd);
    _invited.erase(fd);
}
bool Channel::hasMember(int fd) const {
    return _members.find(fd) != _members.end();
}

void Channel::addOperator(int fd) { _operators.insert(fd); }
void Channel::removeOperator(int fd) { _operators.erase(fd); }
bool Channel::isOperator(int fd) const {
    return _operators.find(fd) != _operators.end();
}

void Channel::setInviteOnly(bool value) { _inviteOnly = value; }
void Channel::setTopicLock(bool value) { _topicLock = value; }
void Channel::setKey(const std::string& key) { _key = key; }
void Channel::setLimit(int limit)  { _limit = limit; }

bool Channel::isInviteOnly() const { return _inviteOnly; }
bool Channel::isTopicLocked() const { return _topicLock; }
const std::string& Channel::getKey() const { return _key; }
int Channel::getLimit() const { return _limit; }

void Channel::invite(int fd) { _invited.insert(fd); }
bool Channel::isInvited(int fd) const {
    return _invited.find(fd) != _invited.end();
}

const std::set<int>& Channel::getMembers() const {
    return _members;
}