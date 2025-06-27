#include "../CommandHandler.hpp"
#include "../../server/Server.hpp"
#include "../../channel/Channel.hpp"
#include <iostream>
#include <sstream>

void password(Client *client, std::istringstream &iss)
{
    std::string givenPassword;
    iss >> givenPassword;

    if (givenPassword.empty())
    {
        return;
    }

    else if (givenPassword != client->getServer()->getPassword())
    {
        client->setDisconnected(true);
        return;
    }
    else
    {
        client->markPasswordGiven();
        client->setDisconnected(false);
    }

}
