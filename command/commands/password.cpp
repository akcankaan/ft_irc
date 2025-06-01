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
        std::cout << "Client " << client->getFd() << " gave empty password." << std::endl;
        return;
    }
    
    else if (givenPassword != client->getServer()->getPassword())
    {
        std::cout << "Client " << client->getFd() << " gave WRONG password." << std::endl;
        client->setDisconnected(true);
        return;
    }
    else
        client->markPasswordGiven();
    
    std::cout << "Client " << client->getFd() << " gave correct password." << std::endl;
}