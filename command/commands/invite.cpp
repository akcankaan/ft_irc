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

    if (nick.empty() && chanName.empty())
    {
        std::cout << "INVITE syntax: INVITE <nick> <#channel>" << std::endl;
        std::string warning = ":@localhost 421 " + client->getNickname() +
                  " :INVITE syntax: INVITE <nick> <#channel>\r\n";
        send(client->getFd(), warning.c_str(), warning.length(), 0);
        return;
    }

    Server *server = client->getServer();
    Channel *channel = server->getChannelMap()[chanName];
    if (!channel)
    {
        std::cout << "Channel not found" << std::endl;
        std::string warning = ":@localhost 403 " + client->getNickname() +
                  " :Channel not found\r\n";
        send(client->getFd(), warning.c_str(), warning.length(), 0);
        return;
    }

    if (!channel->isOperator(client->getNickname()))
    {
        std::cout << "You are not channel operator" << std::endl;
        std::string warning = ":@localhost 481 " + client->getNickname() +
                  " :You are not channel operator\r\n";
        send(client->getFd(), warning.c_str(), warning.length(), 0);
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
