#include "../CommandHandler.hpp"
#include <iostream>
#include <sstream>

void user(Client *client, std::istringstream &iss)
{
    std::string username;
    iss >> username;
    client->setUsername(username);
    std::cout << "Client " << client->getFd() << " set username to " << username << std::endl;
}