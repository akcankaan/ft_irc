#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>
#include <map>

class Channel {
    private:
        std::string _name;
        std::string _topic;
        std::string _key;
        int _limit;
        bool _inviteOnly;
        bool _topicLock;

        std::set<int> _members;
        std::set<int> _operators;
        std::set<int> _invited;
    public:
        Channel(const std::string& name);

        const std::string& getName() const;
        const std::string& getTopic() const;
        void setTopic(const std::string& topic);

        void addMember(int fd);
        void removeMember(int fd);
        bool hasMember(int fd) const;

        void addOperator(int fd);
        void removeOperator(int fd);
        bool isOperator(int fd) const;

        void setInviteOnly(bool value);
        void setTopicLock(bool value);
        void setKey(const std::string& key);
        void setLimit(int limit);

        bool isInviteOnly() const;
        bool isTopicLocked() const;
        const std::string& getKey() const;
        int getLimit() const;

        void invite(int fd);
        bool isInvited(int fd) const;

        const std::set<int>& getMembers() const;
};

#endif