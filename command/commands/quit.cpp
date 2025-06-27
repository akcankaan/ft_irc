#include "../CommandHandler.hpp"
#include "../../server/Server.hpp"
#include "../../channel/Channel.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>


void quit(Client *client, std::istringstream &iss)
{
    std::string dummy;
    iss >> dummy;

    std::string msg;
    std::getline(iss, msg);

    if (client->shouldDisconnect())
        return;

    if (!msg.empty() && msg[0] == ' ')
        msg.erase(0, 1);
    if (!msg.empty() && msg[0] == ':')
        msg.erase(0, 1);
    if (msg.empty())
        msg = "Client exited";

    Server *server = client->getServer();
    std::map<std::string, Channel *> &channels = server->getChannelMap();

    std::string quitMsg = ":" + client->getNickname() + "!" + client->getUsername() +
                          "@localhost QUIT :" + msg + "\r\n";


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

        bool wasOperator = channel->isOperator(client->getNickname());

        channel->removeClient(client);

        channel->broadcast(quitMsg, client);

        if (wasOperator && !channel->getClients().empty())
        {

            Client* newOp = channel->getClients().back();
            channel->addOperator(newOp->getNickname());

            std::string modeMsg = ":@localhost MODE " + chanName +
                " +o " + newOp->getNickname() + "\r\n";
            channel->broadcast(modeMsg, NULL);
        }

    }

    server->removeClient(client->getFd());
}
