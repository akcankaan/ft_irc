#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include "../client/Client.hpp"
#include <string>
#include <map>
#include <set>

class CommandHandler {
    public:
        static void handleCommand(Client *client, const std::string &raw);
};

#endif