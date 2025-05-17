#include "CommandHandler.hpp"
#include "../server/Server.hpp"
#include "../channel/Channel.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>

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

        std::string joinPassword;
        iss >> joinPassword;

        // 🔒 invite-only kontrolü
        if (channel->isInviteOnly() && !channel->isInvited(client)) {
            send(client->getFd(), "Channel is invite-only\r\n", 26, 0);
            return;
        }

        // 🔑 şifre kontrolü
        if (channel->hasPassword()) {
            if (joinPassword != channel->getPassword()) {
                send(client->getFd(), "Incorrect channel password\r\n", 29, 0);
                return;
            }
        }

        channel->addClient(client);

        std::cout << "Client " << client->getFd() << " joined " << chanName << std::endl;
    } else if (command == "PRIVMSG") {
        std::string target;
        iss >> target;

        std::string message;
        std::getline(iss, message);
        if (!message.empty() && message[0] == ' ')
            message.erase(0, 1);

        if (target.empty() || message.empty()) {
            std::cout << "Client " << client->getFd() << " sent invalid PRIVMSG." << std::endl;
            return;
        }

        Server *server = client->getServer();
        std::map<std::string, Channel*> &channels = server->getChannelMap();
        
        if (channels.find(target) == channels.end()) {
            std::cout << "Channel not found: " << target << std::endl;
            return;
        }

        Channel *channel = channels[target];
        std::string fullMessage = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
        channel->broadcast(fullMessage, client);
        
        std::cout << "Client " << client->getFd() << " sent to " << target << ": " << message << std::endl;
    } else if (command == "KICK") {
        std::string chanName, targetNick;
        iss >> chanName >> targetNick;

        std::string reason;
        std::getline(iss, reason);
        if (!reason.empty() && reason[0] == ' ')
            reason.erase(0, 1);
        if (reason.empty())
            reason = "No reason";

        Server *server = client->getServer();
        Channel *channel = server->getChannelMap()[chanName];
        if (!channel) {
            std::cout << "Channel not found: " << chanName << std::endl;
            return;
        }

        Client *target = NULL;
        const std::map<int, Client*> &clients = server->getClientMap();
        for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
            if (it->second->getNickname() == targetNick) {
                target = it->second;
                break;
            }
        }
        
        if (!target) {
            std::cout << "Target not found: " << targetNick << std::endl;
            return;
        }

        channel->kickClient(client, target, reason);
        std::cout << "Client " << client->getFd() << " kicked " << targetNick << " from " << chanName << std::endl;
    } else if (command == "TOPIC") {
        std::string chanName;
        iss >> chanName;

        if (chanName.empty()) {
            send(client->getFd(), "No channel given\r\n", 19, 0);
            return;
        }

        Server *server = client->getServer();
        Channel *channel = server->getChannelMap()[chanName];
        if (!channel) {
            send(client->getFd(), "Channel not found\r\n", 20, 0);
            return;
        }

        std::string newTopic;
        std::getline(iss, newTopic);
        if (!newTopic.empty() && newTopic[0] == ' ')
            newTopic.erase(0, 1);
        
        if (newTopic.empty()) {
            std::string response = "Current topic: " + channel->getTopic() + "\r\n";
            send(client->getFd(), response.c_str(), response.length(), 0);
            return;
        }
        if (!channel->isOperator(client)) {
            send(client->getFd(), "You're not channel operator\r\n", 30, 0);
            return;
        }

        channel->setTopic(newTopic);
        
        std::string notify = ":" + client->getNickname() + " TOPIC " + chanName + " :" + newTopic + "\r\n";
        channel->broadcast(notify, NULL);
        std::cout << "Topic of " << chanName << " set to:" << newTopic << std::endl;
    } else if (command == "INVITE") {
            std::string nick, chanName;
            iss >> nick >> chanName;

            if (nick.empty() || chanName.empty()) {
                send(client->getFd(), "INVITE syntax: INVITE <nick> <#channel>\r\n", 43, 0);
                return;
            }

            Server *server = client->getServer();
            Channel *channel = server->getChannelMap()[chanName];
            if (!channel) {
                send(client->getFd(), "Channel not found\r\n", 20, 0);
                return;
            }

            if (!channel->isOperator(client)) {
                send(client->getFd(), "You are not channel operator\r\n", 31, 0);
                return;
            }

            Client *target = NULL;
            const std::map<int, Client*> &clients = server->getClientMap();
            for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
                if (it->second->getNickname() == nick) {
                    target = it->second;
                    break;
                }
            }

            channel->inviteClient(target);
            
            std::string notice = ":" + client->getNickname() + " INVITE " + nick + " :" + chanName + "\r\n";
            send(target->getFd(), notice.c_str(), notice.length(), 0);
            send(client->getFd(), "User invited\r\n", 15, 0);

            std::cout << "Client " << client->getFd() << " invited " << nick << " to " << chanName << std::endl;
    } else if (command == "MODE") {
    std::string chanName, modeStr;
    iss >> chanName >> modeStr;

    if (chanName.empty() || modeStr.empty()) {
        send(client->getFd(), "Usage: MODE <#channel> [+/-mode]\r\n", 35, 0);
        return;
    }

    Server *server = client->getServer();
    std::map<std::string, Channel*> &channels = server->getChannelMap();

    // ✅ Kanal var mı kontrolü
    if (channels.find(chanName) == channels.end()) {
        send(client->getFd(), "Channel not found\r\n", 20, 0);
        return;
    }

    Channel *channel = channels[chanName];

    // ✅ Kullanıcı operatör mü?
    if (!channel->isOperator(client)) {
        send(client->getFd(), "You are not channel operator\r\n", 31, 0);
        return;
    }

    // ✅ Mod işlemleri
    if (modeStr == "+i") {
        channel->setInviteOnly(true);
        send(client->getFd(), "Invite-only mode set (+i)\r\n", 28, 0);
    } else if (modeStr == "-i") {
        channel->setInviteOnly(false);
        send(client->getFd(), "Invite-only mode removed (-i)\r\n", 33, 0);
    } else if (modeStr == "+k") {
        std::string key;
        iss >> key;

            if (key.empty()) {
                send(client->getFd(), "Password required for +k\r\n", 27, 0);
                return;
            }

            channel->setPassword(key);
            send(client->getFd(), "Channel password set (+k)\r\n", 28, 0);
        }
        else if (modeStr == "-k") {
            channel->clearPassword();
            send(client->getFd(), "Channel password removed (-k)\r\n", 32, 0);
        }
        else {
            send(client->getFd(), "Unsupported MODE\r\n", 19, 0);
        }
    } else {
        std::cout << "Unknown command from client " << client->getFd() << ": " << raw << std::endl;
    }
}