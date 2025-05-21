#include "../CommandHandler.hpp"
#include "../../server/Server.hpp"
#include "../../channel/Channel.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <cstdlib>
#include <string.h>

void password(Client *client, std::istringstream &iss)
{
    std::string givenPassword;
    iss >> givenPassword;

        // password boşsa veya eksikse
    if (givenPassword.empty()) 
    {
        std::cout << "Client " << client->getFd() << " gave empty password." << std::endl;
        return;
    }

        // Doğrulama burada yapılır, örnek olarak doğru kabul edelim:
    client->markPasswordGiven();
    std::cout << "Client " << client->getFd() << " gave correct password." << std::endl;
    if (!client->hasGivenPassword()) {
        std::cout << "Client " << client->getFd() << " tried command  without PASS." << std::endl;
        return;
    }
}