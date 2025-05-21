#include "CommandHandler.hpp"
#include "../server/Server.hpp"
#include "../channel/Channel.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <cstdlib>

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
            send(client->getFd(), "Invalid channel name\r\n", 23, 0);
            return;
        }

        Server *server = client->getServer();
        Channel *channel = server->getOrCreateChannel(chanName);

        // 🔒 invite-only kontrolü
        if (channel->isInviteOnly() && !channel->isInvited(client)) {
            send(client->getFd(), "Channel is invite-only\r\n", 26, 0);
            return;
        }

        // 🔒 şifre kontrolü
        std::string joinPassword;
        iss >> joinPassword;
        if (channel->hasPassword()) {
            if (joinPassword != channel->getPassword()) {
                send(client->getFd(), "Incorrect channel password\r\n", 29, 0);
                return;
            }
        }

        // 🔒 kullanıcı limiti kontrolü
        if (channel->isFull()) {
            send(client->getFd(), "Channel is full\r\n", 18, 0);
            return;
        }

        // ✅ kanala ekle
        channel->addClient(client);

        std::cout << "Client " << client->getFd() << " joined " << chanName << std::endl;

        // ✅ JOIN bildirimi (herkese)
        std::string joinNotice = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost JOIN :" + chanName + "\r\n";
        const std::vector<Client*> &clients = channel->getClients();
        for (size_t i = 0; i < clients.size(); ++i) {
            send(clients[i]->getFd(), joinNotice.c_str(), joinNotice.length(), 0);  // ✅ send, NOT end
        }

        // ✅ TOPIC bilgisi
        std::string topicMsg = ":ircserv 332 " + client->getNickname() + " " + chanName + " :" + channel->getTopic() + "\r\n";
        send(client->getFd(), topicMsg.c_str(), topicMsg.length(), 0);

        // ✅ NAMES listesi
        std::string nameList = ":ircserv 353 " + client->getNickname() + " = " + chanName + " :";
        for (size_t i = 0; i < clients.size(); ++i) {
            nameList += clients[i]->getNickname() + " ";
        }
        nameList += "\r\n";
        send(client->getFd(), nameList.c_str(), nameList.length(), 0);

        // ✅ NAMES bitiş bildirimi
        std::string endNames = ":ircserv 366 " + client->getNickname() + " " + chanName + " :End of /NAMES list.\r\n";
        send(client->getFd(), endNames.c_str(), endNames.length(), 0);
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
        std::string fullMessage = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost PRIVMSG " + target + " :" + message + "\r\n";
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
        std::string kickMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost KICK " +
                      chanName + " " + targetNick + " :" + reason + "\r\n";
        channel->broadcast(kickMsg, client);

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
        if (channel->isTopicRestricted() && !channel->isOperator(client)) {
            send(client->getFd(), "You're not allowed to change the topic\r\n", 40, 0);
            return;
        }

        channel->setTopic(newTopic);

        std::string notify = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost TOPIC " +
                                chanName + " :" + newTopic + "\r\n";
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

            std::string notice = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost INVITE " +
            nick + " :" + chanName + "\r\n";
            send(target->getFd(), notice.c_str(), notice.length(), 0);

            send(client->getFd(), "User invited\r\n", 15, 0);

            std::cout << "Client " << client->getFd() << " invited " << nick << " to " << chanName << std::endl;
    } else if (command == "MODE") {
        std::string chanName, modeStr, param;
        iss >> chanName >> modeStr >> param;

        if (chanName.empty() || modeStr.empty()) {
            send(client->getFd(), "Usage: MODE <#channel> [+/-mode] [param]\r\n", 45, 0);
            return;
        }

        Server *server = client->getServer();
        std::map<std::string, Channel*> &channels = server->getChannelMap();

        if (channels.find(chanName) == channels.end()) {
            send(client->getFd(), "Channel not found\r\n", 20, 0);
            return;
        }

        Channel *channel = channels[chanName];

        if (!channel->isOperator(client)) {
            send(client->getFd(), "You are not channel operator\r\n", 31, 0);
            return;
        }

        std::string modeResponse = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + chanName + " ";

        if (modeStr == "+i") {
            channel->setInviteOnly(true);
            modeResponse += "+i\r\n";
            channel->broadcast(modeResponse, NULL);
        } else if (modeStr == "-i") {
            channel->setInviteOnly(false);
            modeResponse += "-i\r\n";
            channel->broadcast(modeResponse, NULL);
        } else if (modeStr == "+t") {
            channel->setTopicRestrict(true);
            modeResponse += "+t\r\n";
            channel->broadcast(modeResponse, NULL);
        } else if (modeStr == "-t") {
            channel->setTopicRestrict(false);
            modeResponse += "-t\r\n";
            channel->broadcast(modeResponse, NULL);
        } else if (modeStr == "+k") {
            if (param.empty()) {
                send(client->getFd(), "Password required for +k\r\n", 28, 0);
                return;
            }
            channel->setPassword(param);
            modeResponse += "+k " + param + "\r\n";
            channel->broadcast(modeResponse, NULL);
        } else if (modeStr == "-k") {
            channel->setPassword("");
            modeResponse += "-k\r\n";
            channel->broadcast(modeResponse, NULL);
        } else if (modeStr == "+l") {
            if (param.empty()) {
                send(client->getFd(), "User limit required for +l\r\n", 30, 0);
                return;
            }
            int limit = std::atoi(param.c_str());
            channel->setUserLimit(limit);
            modeResponse += "+l " + param + "\r\n";
            channel->broadcast(modeResponse, NULL);
        } else if (modeStr == "-l") {
            channel->setUserLimit(0);
            modeResponse += "-l\r\n";
            channel->broadcast(modeResponse, NULL);
        } else {
            send(client->getFd(), "Unknown mode\r\n", 15, 0);
            return;
        }

        std::cout << "Client " << client->getFd() << " set mode " << modeStr << " on " << chanName << std::endl;
    }
}
