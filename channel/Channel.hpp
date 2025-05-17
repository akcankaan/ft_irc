#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "../client/Client.hpp"
#include <string>
#include <vector>
#include <set>

class Channel {
    private:
        std::string _name;
        std::vector<Client*> _clients;
        std::string _topic;
        std::set<std::string> _invited;

    public:
        Channel(const std::string &name);
        ~Channel();

        const std::string &getName() const;

        void addClient(Client *client);
        bool hasClient(Client *client) const;

        void broadcast(const std::string &message, Client *sender);

        void removeClient(Client *client);
        void kickClient(Client *by, Client *target, const std::string &reason);
        bool isOperator(Client *client) const;
        void setOperator(Client *client);
        const std::string &getTopic() const;
        void setTopic(const std::string &topic);
        void inviteClient(Client *target);
        bool isInvited(const Client *client) const;
};

#endif