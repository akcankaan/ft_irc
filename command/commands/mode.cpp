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
    std::string chanName, modeStr;
    iss >> chanName >> modeStr;

    if (chanName.empty() && modeStr.empty())
    {
        std::cout << "Usage: MODE <#channel> [+/-mode] [param]" << std::endl;
        std::string warning = ":@localhost 421 " + client->getNickname() +
                " :Usage: MODE <#channel> [+/-mode] [param]\r\n";
        send(client->getFd(), warning.c_str(), warning.length(), 0);
        return;
    }

    Server *server = client->getServer();
    std::map<std::string, Channel*> &channels = server->getChannelMap();

    if (channels.find(chanName) == channels.end())
    {
        std::cout << "Channel not found" << std::endl;
        std::string warning = ":@localhost 403 " + client->getNickname() +
                " :Channel not found\r\n";
        send(client->getFd(), warning.c_str(), warning.length(), 0);
        return;
    }

    Channel *channel = channels[chanName];

    if (!channel->isOperator(client->getNickname()))
    {
        std::cout << "You are not channel operator" << std::endl;
        std::string warning = ":@localhost 482 " + client->getNickname() +
                " :You are not channel operator\r\n";
        send(client->getFd(), warning.c_str(), warning.length(), 0);
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
        std::string param;
        iss >> param;
        if (param.empty())
        {
            std::cout << "Password required for +k" << std::endl;
            std::string warning = ":@localhost 421 " + client->getNickname() +
                    " :Password required for +k\r\n";
            send(client->getFd(), warning.c_str(), warning.length(), 0);
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
        std::string param;
        iss >> param;
        if (param.empty())
        {
            std::cout << "User limit required for +l" << std::endl;
            std::string warning = ":@localhost 421 " + client->getNickname() +
                    " :User limit required for +l\r\n";
            send(client->getFd(), warning.c_str(), warning.length(), 0);
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
    } else if(modeStr == "+o"){
        std::string nickname;
        iss >> nickname;
        if (iss.fail() || !iss.eof())
        {
            std::cout << "Wrong input." << std::endl;
            std::string warning = ":@localhost 421 " + client->getNickname() +
                    " :" + "Wrong input\r\n";
            send(client->getFd(), warning.c_str(), warning.length(), 0);
            return;
        }
        if(channel->isOperator(nickname))
        {
            std::cout << nickname << " : already operator" << std::endl;
            std::string warning = ":@localhost 421 " + client->getNickname() +
                    " :"+ nickname +" :already operator\r\n";
            send(client->getFd(), warning.c_str(), warning.length(), 0);
            return ;
        }

        channel->addOperator(nickname);
        modeResponse = ":" + nickname + "!" + nickname + "@localhost MODE " + chanName + " " + "+o\r\n";
        channel->broadcast(modeResponse, NULL);
    }else if(modeStr == "-o"){
        std::string nickname;
        iss >> nickname;
        if (iss.fail() || !iss.eof())
        {
            std::cout << "Wrong input." << std::endl;
            std::string warning = ":@localhost 421 " + client->getNickname() +
                    " :" + "Wrong input\r\n";
            send(client->getFd(), warning.c_str(), warning.length(), 0);
            return;
        }
        if(!channel->isOperator(nickname))
        {
            std::cout << nickname << " : is not operator" << std::endl;
            std::string warning = ":@localhost 421 " + client->getNickname() +
                    " :"+ nickname +" :is not operator\r\n";
            send(client->getFd(), warning.c_str(), warning.length(), 0);
            return ;
        }

        channel->removeOperator(nickname);
        modeResponse = ":" + nickname + "!" + nickname + "@localhost MODE " + chanName + " " + "-o\r\n";
        channel->broadcast(modeResponse, NULL);
        }else {
        std::cout << "Unknown mode" << std::endl;
        std::string warning = ":@localhost 421 " + client->getNickname() +
                    " :Unknown mode\r\n";
            send(client->getFd(), warning.c_str(), warning.length(), 0);
        return;
    }

    std::cout << "Client " << client->getFd() << " set mode " << modeStr << " on " << chanName << std::endl;
}
