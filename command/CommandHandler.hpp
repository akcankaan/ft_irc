#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include "../client/Client.hpp"
#include <string>
#include <sstream>

class CommandHandler {
    public:
        static void handleCommand(Client *client, const std::string &raw);
};

void password(Client *client, std::istringstream &iss);
void nick(Client *client, std::istringstream &iss);
void user(Client *client, std::istringstream &iss);
void join(Client *client, std::istringstream &iss);
void privmsg(Client *client, std::istringstream &iss);
void kick(Client *client, std::istringstream &iss);
void topic (Client *client, std::istringstream &iss);
void invite (Client *client, std::istringstream &iss);
void mode (Client *client, std::istringstream &iss);
void quit(Client *client, std::istringstream &iss);
void part(Client *client, std::istringstream &iss);

#endif
