#include "../CommandHandler.hpp"
#include "../../server/Server.hpp"
#include "../../channel/Channel.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>


void topic (Client *client, std::istringstream &iss)
{
    std::string chanName;
    iss >> chanName;
    if (client->shouldDisconnect())
        return;
    if (chanName.empty())
    {
        std::string warning = ":@localhost 421 " + client->getNickname() +
                    " :" + "No channel given\r\n";
        send(client->getFd(), warning.c_str(), warning.length(), 0);
        return;
    }

    Server *server = client->getServer();
    Channel *channel = server->getChannelMap()[chanName];
    if (!channel)
    {
        std::string warning = ":@localhost 403 " + client->getNickname() +
                " :" + "Channel not found\r\n";
        send(client->getFd(), warning.c_str(), warning.length(), 0);
        return;
    }

    std::string newTopic;
    std::getline(iss, newTopic);
    if (!newTopic.empty() && newTopic[0] == ' ' && newTopic[1] == ':')
        newTopic.erase(0, 2);

    if (newTopic.empty())
    {
        std::string response = "Current topic: " + channel->getTopic() + "\r\n";
        send(client->getFd(), response.c_str(), response.length(), 0);
        return;
    }
    if (channel->isTopicRestricted() && !channel->isOperator(client->getNickname()))
    {
        std::string warning = ":@localhost 482 " + client->getNickname() +
                " :" + "You're not allowed to change the topic\r\n";
        send(client->getFd(), warning.c_str(), warning.length(), 0);
        return;
    }

    channel->setTopic(newTopic);

    std::string notify = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost TOPIC " +
                            chanName + " :" + newTopic + "\r\n";
    channel->broadcast(notify, NULL);

}
