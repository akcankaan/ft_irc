#include "../CommandHandler.hpp"
#include <iostream>
#include <sstream>

void nick(Client *client, std::istringstream &iss)
{
    std::string nickname;
    iss >> nickname;
    client->setNickname(nickname);
    std::cout << "Client " << client->getFd() << " set nickname to " << nickname << std::endl;
}