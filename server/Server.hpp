#ifndef SERVER_HPP
#define SERVER_HPP

#include "../client/Client.hpp"
#include "../command/CommandHandler.hpp"
#include "../channel/Channel.hpp"
#include <map>
#include <vector>
#include <poll.h>

class Server {
    private:
        int _port;
        std::string _password;

        int _serverSocket;
        std::map<int, Client*> _clients;
        std::vector<struct pollfd> _pollFds;
        std::map<std::string, Channel*> _channels;

        void addClient(int client_fd);

        public:
        Server(int port, const std::string& password);
        ~Server();

        void run();
        Channel* getOrCreateChannel(const std::string &name);
        std::map<std::string, Channel*> &getChannelMap();
        const std::map<int, Client*> &getClientMap() const;
        std::string getPassword() const;
        void removeClient(int client_fd);
        bool isNicknameInUse(const std::string &nickname) const;

};

#endif
