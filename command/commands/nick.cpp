#include "../CommandHandler.hpp"
#include "../../server/Server.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <sys/socket.h>

bool isSpecial(char c)
{
    return c == '[' || c == ']' || c == '-' || c == '_' || c == '\'' || c == '\\' || c == '|' || c == '^' || c == '{' || c == '}';
}

void nick(Client *client, std::istringstream &iss)
{
    std::string nickname;
    static int i = 0;
    static std::string oldNick;
    iss >> nickname;
    if (client->shouldDisconnect())
        return;

    if (nickname == client->getNickname())
        return;

    size_t len = nickname.size();
    if (len < 2 || len > 9)
    {
        std::string msg = ":server 432 * " + nickname + " :Erroneous nickname\r\n";
        send(client->getFd(), msg.c_str(), msg.length(), 0);
        if (i == 0)
            oldNick = nickname;
        i = 1;
        return ;
    }

    unsigned char c0 = static_cast<unsigned char>(nickname[0]);
    if (!std::isalpha(c0) && !isSpecial(static_cast<char>(c0)))
    {
        std::string msg = ":server 432 * " + nickname + " :Erroneous nickname\r\n";
        send(client->getFd(), msg.c_str(), msg.length(), 0);
        if (i == 0)
            oldNick = nickname;
        i = 1;
        return ;
    }


    for (size_t j = 1; j < len; ++j)
    {
        unsigned char ci = static_cast<unsigned char>(nickname[j]);
        if (!std::isalnum(ci) && !isSpecial(static_cast<char>(ci)))
        {
            std::string msg = ":server 432 * " + nickname + " :Erroneous nickname\r\n";
            send(client->getFd(), msg.c_str(), msg.length(), 0);
            if (i == 0)
                oldNick = nickname;
            i = 1;
            return ;
        }
    }

    Server* server = client->getServer();

    if (server->isNicknameInUse(nickname)) {
        if (i == 0)
            oldNick = nickname;
        i = 1;
        std::string msg = ":server 433 * " + nickname + " :Nickname is already in use\r\n";
        send(client->getFd(), msg.c_str(), msg.length(), 0);
        return;
    }
    if (client->getNickname().length() != 0)
        oldNick = client->getNickname();
    client->setNickname(nickname);

    std::string msg = ":" + oldNick + "!" + client->getUsername() + "@localhost NICK :" + nickname + "\r\n";

    std::map<std::string, Channel *> &channels = server->getChannelMap();
    std::vector<Channel*> userChannels;
    for (std::map<std::string, Channel *>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        Channel* channel = it->second;
        const std::vector<Client*>& clist = channel->getClients();
        if (std::find(clist.begin(), clist.end(), client) != clist.end())
            userChannels.push_back(channel);
    }
    for (size_t i = 0; i < userChannels.size(); ++i)
    {
        Channel* channel = userChannels[i];
        std::string chanName = channel->getName();
        bool wasOperator = channel->isOperator(oldNick);
        if (wasOperator)
        {
            channel->removeOperator(oldNick);
            channel->addOperator(nickname);
        }
        channel->broadcast(msg, NULL);
    }
    send(client->getFd(), msg.c_str(), msg.length(), 0);
    i = 0;
}
