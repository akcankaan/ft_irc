#include "../CommandHandler.hpp"
#include "../../server/Server.hpp"
#include "../../channel/Channel.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <cstdlib>
#include <string.h>

void kick(Client *client, std::istringstream &iss)
{
    std::string chanName, targetNick;
    iss >> chanName >> targetNick;

    std::string reason;
    std::getline(iss, reason);
    if (!reason.empty() && reason[0] == ' ')
        reason.erase(0, 1);
    if (reason.empty())
        reason = "No reason";

    Server *server = client->getServer();
    Channel *channel = server->getChannelMap()[chanName];
    if (!channel) 
    {
        std::cout << "Channel not found: " << chanName << std::endl;
        return;
    }

    Client *target = NULL;
    const std::map<int, Client*> &clients = server->getClientMap();
    for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) 
    {
        if (it->second->getNickname() == targetNick) {
            target = it->second;
            break;
        }
    }

    if (!target) 
    {
        std::cout << "Target not found: " << targetNick << std::endl;
        return;
    }

    channel->kickClient(client, target, reason);
    std::string kickMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost KICK " +
                    chanName + " " + targetNick + " :" + reason + "\r\n";
    channel->broadcast(kickMsg, client);

    std::cout << "Client " << client->getFd() << " kicked " << targetNick << " from " << chanName << std::endl;
}