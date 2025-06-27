#include "../CommandHandler.hpp"
#include "../../server/Server.hpp"
#include "../../channel/Channel.hpp"
#include <iostream>
#include <sstream>

void privmsg(Client *client, std::istringstream &iss)
{
    std::string target;
    iss >> target;

    std::string message;
    std::getline(iss, message);
    if (client->shouldDisconnect())
        return;
    if (!message.empty() && message[0] == ' ')
        message.erase(0, 1);
    if (!message.empty() && message[0] == ':')
        message.erase(0, 1);

    if (target.empty() || message.empty()) {
        return;
    }

    Server *server = client->getServer();
    std::map<std::string, Channel*> &channels = server->getChannelMap();

    if (channels.find(target) == channels.end()) {
        return;
    }

    Channel *channel = channels[target];

    if (!channel->isClientInChannel(client)) {
        return;
    }
    std::string fullMessage = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost PRIVMSG " + target + " :" + message + "\r\n";
    channel->broadcast(fullMessage, client);

}
