#include "CommandHandler.hpp"
#include "../server/Server.hpp"
#include "../channel/Channel.hpp"
#include <sstream>
#include <algorithm>

void CommandHandler::handleCommand(Client *client, const std::string &raw)
{
    std::istringstream iss(raw);
    std::string command;
    iss >> command;

    std::transform(command.begin(), command.end(), command.begin(), ::toupper);

    if (client->isAuthenticated() == false)
    {
        if (command == "PASS")
            password(client, iss);
        else if (command == "NICK")
            nick(client, iss);
        else if (command == "USER")
            user(client, iss);
        else if (command == "QUIT")
            quit(client, iss);

        if (!client->getNickname().empty() && !client->getUsername().empty() && client->hasGivenPassword())
        {
            client->authenticate();
        }
        return;
    }

    if (command == "PASS")
        password(client, iss);
    else if (command == "NICK")
        nick(client, iss);
    else if (command == "USER")
        user(client, iss);
    else if (command == "JOIN")
        join(client, iss);
    else if (command == "PRIVMSG")
        privmsg(client, iss);
    else if (command == "KICK")
        kick(client, iss);
    else if (command == "TOPIC")
        topic(client, iss);
    else if (command == "INVITE")
        invite(client, iss);
    else if (command == "MODE")
        mode(client, iss);
    else if (command == "QUIT")
        quit(client, iss);
    else if (command == "PART")
        part(client, iss);
    else
        return;
}