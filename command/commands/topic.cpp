#include "../CommandHandler.hpp"
#include "../../server/Server.hpp"
#include "../../channel/Channel.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <cstdlib>
#include <string.h>

void topic (Client *client, std::istringstream &iss)
{
    std::string chanName;
    iss >> chanName;

    if (chanName.empty()) 
    {
        const char *msg = "No channel given\r\n";
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
    if (channel->isTopicRestricted() && !channel->isOperator(client)) 
    {
        const char *msg = "You're not allowed to change the topic\r\n";
        send(client->getFd(), msg, strlen(msg), 0);
        return;
    }

    channel->setTopic(newTopic);

    std::string notify = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost TOPIC " +
                            chanName + " :" + newTopic + "\r\n";
    channel->broadcast(notify, NULL);

    std::cout << "Topic of " << chanName << " set to:" << newTopic << std::endl;
}