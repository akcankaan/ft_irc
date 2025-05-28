#include "../CommandHandler.hpp"
#include "../../server/Server.hpp"
#include "../../channel/Channel.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <cstdlib>
#include <string.h>

void privmsg(Client *client, std::istringstream &iss)
{
    std::string target;
    iss >> target;

    std::string message;
    std::getline(iss, message);

        // Fazlalıkları temizle
    if (!message.empty() && message[0] == ' ')
        message.erase(0, 1);
    if (!message.empty() && message[0] == ':')
        message.erase(0, 1);  // ÇİFT :: problemini engeller

    if (target.empty() || message.empty()) {
        std::cout << "Client " << client->getFd() << " sent invalid PRIVMSG." << std::endl;
        return;
    }

    Server *server = client->getServer();
    std::map<std::string, Channel*> &channels = server->getChannelMap();

    if (channels.find(target) == channels.end()) {
        std::cout << "Channel not found: " << target << std::endl;
        return;
    }

    Channel *channel = channels[target];

    if (!channel->isClientInChannel(client)) {
        send(client->getFd(), "2\r\n", 4, 0);
        return;
    }
        // ✅ Doğru format: sadece tek ':' kullanılmalı!
    std::string fullMessage = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost PRIVMSG " + target + " :" + message + "\r\n";
    channel->broadcast(fullMessage, client);

    std::cout << "Client " << client->getFd() << " sent to " << target << ": " << message << std::endl;
}
