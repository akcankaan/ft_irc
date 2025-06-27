#include "../CommandHandler.hpp"
#include "../../server/Server.hpp"
#include "../../channel/Channel.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>

void kick(Client *client, std::istringstream &iss)
{
    std::string chanName, targetNick;
    iss >> chanName >> targetNick;
    if (client->shouldDisconnect())
        return;
    std::string reason;
    std::getline(iss, reason);
    if (!reason.empty() && reason[0] == ' ' && reason[1] == ':')
        reason.erase(0, 2);
    if (reason.empty())
        reason = "No reason";
    Server *server = client->getServer();
    Channel *channel = server->getChannelMap()[chanName];
    if (!(channel->isOperator(client->getNickname())))
    {

        std::string warning = ":@localhost 481 " + client->getNickname() +
                  " :You are not channel operator\r\n";
        send(client->getFd(), warning.c_str(), warning.length(), 0);
        return;
    }
    if (!channel)
    {
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
        std::string warning = ":@localhost 441 " + client->getNickname() +
                  " :User not found: " + targetNick +"\r\n";
        send(client->getFd(), warning.c_str(), warning.length(), 0);
        return;
    }
    if(channel->isOperator(targetNick)){
        std::string warning = ":@localhost 483 " + client->getNickname() +
                  " :Cannot kick " + targetNick +" :an operator\r\n";
        send(client->getFd(), warning.c_str(), warning.length(), 0);
        return ;
    }
    channel->kickClient(client, target, reason);
    std::string kickMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost KICK " +
                    chanName + " " + targetNick + " :" + reason + "\r\n";
    channel->broadcast(kickMsg, client);

}
