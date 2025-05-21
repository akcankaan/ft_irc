#include "../CommandHandler.hpp"
#include "../../server/Server.hpp"
#include "../../channel/Channel.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <cstdlib>
#include <string.h>

void user(Client *client, std::istringstream &iss)
{
    std::string username;
    iss >> username;
    client->setUsername(username);
    std::cout << "Client " << client->getFd() << " set username to " << username << std::endl;
}