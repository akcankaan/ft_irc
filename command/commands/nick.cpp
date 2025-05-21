#include "../CommandHandler.hpp"
#include "../../server/Server.hpp"
#include "../../channel/Channel.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <cstdlib>
#include <string.h>

void nick(Client *client, std::istringstream &iss)
{
    std::string nickname;
    iss >> nickname;
    client->setNickname(nickname);
    std::cout << "Client " << client->getFd() << " set nickname to " << nickname << std::endl;
}