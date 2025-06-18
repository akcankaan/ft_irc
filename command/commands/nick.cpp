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

    Server* server = client->getServer();
    if (server->isNicknameInUse(nickname)) {
        std::string msg = ":server 433 * " + nickname + " :Nickname is already in use\r\n";
        send(client->getFd(), msg.c_str(), msg.length(), 0);
        return;
    }

    client->setNickname(nickname);
    std::cout << "Client " << client->getFd() << " set nickname to " << nickname << std::endl;
}
