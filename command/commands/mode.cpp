#include "../CommandHandler.hpp"
#include "../../server/Server.hpp"
#include "../../channel/Channel.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <cstdlib>
#include <string.h>

void mode (Client *client, std::istringstream &iss)
{
    std::string chanName, modeStr, param;
    iss >> chanName >> modeStr >> param;

    if (chanName.empty() || modeStr.empty()) 
    {
        const char *msg = "Usage: MODE <#channel> [+/-mode] [param]\r\n";
        send(client->getFd(), msg, strlen(msg), 0);
        return;
    }

    Server *server = client->getServer();
    std::map<std::string, Channel*> &channels = server->getChannelMap();

    if (channels.find(chanName) == channels.end()) 
    {
        const char *msg = "Channel not found\r\n";
        send(client->getFd(), msg, strlen(msg), 0);
        return;
    }

    Channel *channel = channels[chanName];

    if (!channel->isOperator(client)) 
    {
        const char *msg ="You are not channel operator\r\n";
        send(client->getFd(), msg, strlen(msg), 0);
        return;
    }

    std::string modeResponse = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + chanName + " ";

    if (modeStr == "+i") {
        channel->setInviteOnly(true);
        modeResponse += "+i\r\n";
        channel->broadcast(modeResponse, NULL);
    } else if (modeStr == "-i") {
        channel->setInviteOnly(false);
        modeResponse += "-i\r\n";
        channel->broadcast(modeResponse, NULL);
    } else if (modeStr == "+t") {
        channel->setTopicRestrict(true);
        modeResponse += "+t\r\n";
        channel->broadcast(modeResponse, NULL);
    } else if (modeStr == "-t") {
        channel->setTopicRestrict(false);
        modeResponse += "-t\r\n";
        channel->broadcast(modeResponse, NULL);
    } else if (modeStr == "+k") {
        if (param.empty()) 
        {
            const char *msg = "Password required for +k\r\n";
            send(client->getFd(), msg, strlen(msg), 0);
            return;
        }
        channel->setPassword(param);
        modeResponse += "+k " + param + "\r\n";
        channel->broadcast(modeResponse, NULL);
    } else if (modeStr == "-k") {
        channel->setPassword("");
        modeResponse += "-k\r\n";
        channel->broadcast(modeResponse, NULL);
    } else if (modeStr == "+l") {
        if (param.empty()) 
        {
            const char *msg = "User limit required for +l\r\n";
            send(client->getFd(), msg, strlen(msg), 0);
            return;
        }
        int limit = std::atoi(param.c_str());
        channel->setUserLimit(limit);
        modeResponse += "+l " + param + "\r\n";
        channel->broadcast(modeResponse, NULL);
    } else if (modeStr == "-l") {
        channel->setUserLimit(0);
        modeResponse += "-l\r\n";
        channel->broadcast(modeResponse, NULL);
    } else {
        const char *msg = "Unknown mode\r\n";
        send(client->getFd(), msg, strlen(msg), 0);
        return;
    }

    std::cout << "Client " << client->getFd() << " set mode " << modeStr << " on " << chanName << std::endl;
}