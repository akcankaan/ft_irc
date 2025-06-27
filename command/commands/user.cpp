#include "../CommandHandler.hpp"
#include <iostream>
#include <sstream>

void user(Client *client, std::istringstream &iss)
{
    std::string username;
    iss >> username;
    if (client->shouldDisconnect())
        return;
    client->setUsername(username);
}
