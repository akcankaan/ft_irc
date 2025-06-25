#include "../CommandHandler.hpp"
#include "../../server/Server.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <sys/socket.h>


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
        std::cout << wasOperator << std::endl;
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
