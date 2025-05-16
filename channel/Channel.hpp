#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "../client/Client.hpp"
#include <string>
#include <vector>

class Channel {
    private:
        std::string _name;
        std::vector<Client*> _clients;
    public:
        Channel(const std::string &name);
        ~Channel();

        const std::string &getName() const;

        void addClient(Client *client);
        bool hasClient(Client *client) const;

        void broadcast(const std::string &message, Client *sender);
};

#endif