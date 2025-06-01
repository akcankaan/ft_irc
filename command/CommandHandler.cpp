#include "CommandHandler.hpp"
#include "../server/Server.hpp"
#include "../channel/Channel.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <cstdlib>
#include <string.h>

void CommandHandler::handleCommand(Client *client, const std::string &raw)
{
    std::istringstream iss(raw);
    std::string command;
    iss >> command;
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
        return ;
}
