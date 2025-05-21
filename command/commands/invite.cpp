#include "../CommandHandler.hpp"
#include "../../server/Server.hpp"
#include "../../channel/Channel.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <cstdlib>
#include <string.h>

void invite (Client *client, std::istringstream &iss)
{
    std::string nick, chanName;
    iss >> nick >> chanName;

    if (nick.empty() || chanName.empty()) 
    {
        const char *msg = "INVITE syntax: INVITE <nick> <#channel>\r\n";
        send(client->getFd(), msg, strlen(msg), 0); 
        return;
    }

    Server *server = client->getServer();
    Channel *channel = server->getChannelMap()[chanName];
    if (!channel) 
    {
        const char *msg = "Channel not found\r\n";
        send(client->getFd(), msg, strlen(msg), 0);
        return;
    }

    if (!channel->isOperator(client)) 
    {
        const char *msg = "You are not channel operator\r\n";
        send(client->getFd(), msg, strlen(msg), 0);
        return;
    }

    Client *target = NULL;
    const std::map<int, Client*> &clients = server->getClientMap();
    for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) 
    {
        if (it->second->getNickname() == nick) 
        {
            target = it->second;
            break;
        }
    }

    channel->inviteClient(target);

    std::string notice = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost INVITE " +
    nick + " :" + chanName + "\r\n";
    send(target->getFd(), notice.c_str(), notice.length(), 0);

    const char *msg = "User invited\r\n";
    send(client->getFd(), msg, strlen(msg), 0);

    std::cout << "Client " << client->getFd() << " invited " << nick << " to " << chanName << std::endl;
}