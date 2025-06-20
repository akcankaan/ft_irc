#include "../CommandHandler.hpp"
#include "../../server/Server.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>

void nick(Client *client, std::istringstream &iss)
{
    std::string nickname;
    iss >> nickname;

    if (client->shouldDisconnect())
        return;

    if (nickname == client->getNickname())
        return;

    Server* server = client->getServer();

    if (server->isNicknameInUse(nickname)) {
        std::string msg = ":server 433 * " + nickname + " :Nickname is already in use\r\n";
        send(client->getFd(), msg.c_str(), msg.length(), 0);
        return;
    }

    std::string oldNick = client->getNickname();
    client->setNickname(nickname);

    std::string msg = ":" + oldNick + "!" + client->getUsername() + "@localhost NICK :" + nickname + "\r\n";

    send(client->getFd(), msg.c_str(), msg.length(), 0);

    std::vector<std::string> joined = client->getJoinedChannels();
    for (size_t i = 0; i < joined.size(); ++i) {
        Channel* chan = server->getChannelMap()[joined[i]];
        if (chan)
            chan->broadcast(msg, NULL);
    }

    std::cout << "[DEBUG] Nick change: " << oldNick << " -> " << nickname << std::endl;
}
