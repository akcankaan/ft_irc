#include "CommandHandler.hpp"
#include "../server/Server.hpp"
#include "../channel/Channel.hpp"
#include <iostream>
#include <sstream>

void CommandHandler::handleCommand(Client *client, const std::string &raw) {
    std::istringstream iss(raw);
    std::string command;
    iss >> command;

    if (command == "PASS") {
        std::string givenPassword;
        iss >> givenPassword;

        // password boşsa veya eksikse
        if (givenPassword.empty()) {
            std::cout << "Client " << client->getFd() << " gave empty password." << std::endl;
            return;
        }

        // Doğrulama burada yapılır, örnek olarak doğru kabul edelim:
        client->markPasswordGiven();
        std::cout << "Client " << client->getFd() << " gave correct password." << std::endl;
    } else if (!client->hasGivenPassword()) {
        std::cout << "Client " << client->getFd() << " tried command  without PASS." << std::endl;
        return;
    } else if (command == "NICK") {
        std::string nickname;
        iss >> nickname;
        client->setNickname(nickname);
        std::cout << "Client " << client->getFd() << " set nickname to " << nickname << std::endl;
    } else if (command == "USER") {
        std::string username;
        iss >> username;
        client->setUsername(username);
        std::cout << "Client " << client->getFd() << " set username to " << username << std::endl;
    } else if (command == "JOIN") {
        std::string chanName;
        iss >> chanName;

        if (chanName.empty() || chanName[0] != '#') {
            std::cout << "Client " << client->getFd() << " tried invalid channel name." << std::endl;
            return;
        }

        Server *server = client->getServer();
        Channel *channel = server->getOrCreateChannel(chanName);
        channel->addClient(client);

        std::cout << "Client " << client->getFd() << " joined " << chanName << std::endl;
    } else {
        std::cout << "Unknown command from client " << client->getFd() << ": " << raw << std::endl;
    }
}