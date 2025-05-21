#include "../CommandHandler.hpp"
#include "../../server/Server.hpp"
#include "../../channel/Channel.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <cstdlib>
#include <string.h>

void join(Client *client, std::istringstream &iss)
{
    std::string chanName;
    iss >> chanName;

    if (chanName.empty() || chanName[0] != '#') 
    {
        const char *msg = "Invalid channel name\r\n";
        send(client->getFd(), msg, strlen(msg), 0);
        return;
    }

    Server *server = client->getServer();
    Channel *channel = server->getOrCreateChannel(chanName);

        // 🔒 invite-only kontrolü
    if (channel->isInviteOnly() && !channel->isInvited(client)) 
    {
        const char *msg = "Channel is invite-only\r\n";
        send(client->getFd(), msg, strlen(msg), 0);
        return;
    }

        // 🔒 şifre kontrolü
    std::string joinPassword;
    iss >> joinPassword;
    if (channel->hasPassword()) 
    {
        if (joinPassword != channel->getPassword()) 
        {
            const char *msg = "Incorrect channel password\r\n";
            send(client->getFd(), msg, strlen(msg), 0);
            return;
        }
    }

        // 🔒 kullanıcı limiti kontrolü
    if (channel->isFull()) 
    {
        const char *msg = "Channel is full\r\n";
        send(client->getFd(), msg, strlen(msg), 0); 
        return;
    }

        // ✅ client kanala eklenmeden önce listeyi al
    const std::vector<Client*> &clientsBefore = channel->getClients();

        // ✅ kanala client'ı ekle
    channel->addClient(client);

    std::cout << "Client " << client->getFd() << " joined " << chanName << std::endl;

        // ✅ JOIN bildirimi (herkese + kendine)
    std::string joinMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost JOIN :" + chanName + "\r\n";
    for (size_t i = 0; i < clientsBefore.size(); ++i) {
        send(clientsBefore[i]->getFd(), joinMsg.c_str(), joinMsg.length(), 0);
    }

        // ✅ TOPIC
    std::string topicMsg = ":ircserv 332 " + client->getNickname() + " " + chanName + " :" + channel->getTopic() + "\r\n";
    send(client->getFd(), topicMsg.c_str(), topicMsg.length(), 0);

        // ✅ NAMES
    const std::vector<Client*> &allClients = channel->getClients();
    std::string nameList = ":ircserv 353 " + client->getNickname() + " = " + chanName + " :";
    for (size_t i = 0; i < allClients.size(); ++i) {
        nameList += allClients[i]->getNickname() + " ";
    }
    nameList += "\r\n";
    send(client->getFd(), nameList.c_str(), nameList.length(), 0);

        // ✅ NAMES END
    std::string endNames = ":ircserv 366 " + client->getNickname() + " " + chanName + " :End of /NAMES list.\r\n";
    send(client->getFd(), endNames.c_str(), endNames.length(), 0);
}