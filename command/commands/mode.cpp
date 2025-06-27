#include "../CommandHandler.hpp"
#include "../../server/Server.hpp"
#include "../../channel/Channel.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <cstdlib>


void mode (Client *client, std::istringstream &iss)
{
    std::string chanName, modeStr;
    iss >> chanName >> modeStr;
    if (client->shouldDisconnect())
        return;
    if (chanName.empty() && modeStr.empty())
    {
        std::string warning = ":@localhost 421 " + client->getNickname() +
                " :Usage: MODE <#channel> [+/-mode] [param]\r\n";
        send(client->getFd(), warning.c_str(), warning.length(), 0);
        return;
    }

    Server *server = client->getServer();
    std::map<std::string, Channel*> &channels = server->getChannelMap();

    if (channels.find(chanName) == channels.end())
    {
        std::string warning = ":@localhost 403 " + client->getNickname() +
                " :Channel not found\r\n";
        send(client->getFd(), warning.c_str(), warning.length(), 0);
        return;
    }

    Channel *channel = channels[chanName];

    if (!channel->isOperator(client->getNickname()))
    {
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

        if (iss.fail() || !iss.eof()) {
            std::string warning = ":@localhost 421 " + client->getNickname() + " :Wrong input\r\n";
            send(client->getFd(), warning.c_str(), warning.length(), 0);
            return;
        }

        if(channel->isOperator(nickname)) {
            std::string warning = ":@localhost 421 " + client->getNickname() + " :" + nickname + " :already operator\r\n";
            send(client->getFd(), warning.c_str(), warning.length(), 0);
            return;
        }

        const std::vector<Client*> &clients = channel->getClients();
        bool found = false;
        for (size_t i = 0; i < clients.size(); ++i) {
            if (clients[i] && clients[i]->getNickname() == nickname) {
                found = true;
                break;
            }
        }

        if (!found) {
            std::string warning = ":@localhost 441 " + client->getNickname() + " " + nickname + " " + chanName + " :They aren't on that channel\r\n";
            send(client->getFd(), warning.c_str(), warning.length(), 0);
            return;
        }

        channel->addOperator(nickname);

        modeResponse = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + chanName + " +o " + nickname + "\r\n";
        channel->broadcast(modeResponse, NULL);
    }else if (modeStr == "-o") {
        std::string nickname;
        iss >> nickname;

        if (iss.fail() || !iss.eof()) {
            std::string warning = ":@localhost 421 " + client->getNickname() + " :Wrong input\r\n";
            send(client->getFd(), warning.c_str(), warning.length(), 0);
            return;
        }

        if (!channel->isOperator(nickname)) {
            std::string warning = ":@localhost 482 " + client->getNickname() + " " + chanName + " :They are not channel operator\r\n";
            send(client->getFd(), warning.c_str(), warning.length(), 0);
            return;
        }

        const std::vector<Client*> &clients = channel->getClients();
        bool found = false;
        for (size_t i = 0; i < clients.size(); ++i) {
            if (clients[i] && clients[i]->getNickname() == nickname) {
                found = true;
                break;
            }
        }

        if (!found) {
            std::string warning = ":@localhost 441 " + client->getNickname() + " " + nickname + " " + chanName + " :They aren't on that channel\r\n";
            send(client->getFd(), warning.c_str(), warning.length(), 0);
            return;
        }

        if (channel->operatorCount() <= 1) {
            std::string warning = ":@localhost 482 " + client->getNickname() + " " + chanName + " :Cannot remove the last operator\r\n";
            send(client->getFd(), warning.c_str(), warning.length(), 0);
            return;
        }

        channel->removeOperator(nickname);

        std::string modeResponse = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + chanName + " -o " + nickname + "\r\n";
        channel->broadcast(modeResponse, NULL);
    }
    else
    {
        std::string warning = ":@localhost 421 " + client->getNickname() +
            " :Unknown mode\r\n";
        send(client->getFd(), warning.c_str(), warning.length(), 0);
        return;
    }

}
