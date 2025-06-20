#include "../CommandHandler.hpp"
#include "../../server/Server.hpp"
#include "../../channel/Channel.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>


void quit(Client *client, std::istringstream &iss)
{
	std::string chanName;
	iss >> chanName;

	std::string msg;
	std::getline(iss, msg);
	if (client->shouldDisconnect())
		return;
	if (!msg.empty() && msg[0] == ' ')
		msg.erase(0, 1);

	if (!msg.empty() && msg[0] == ':')
		msg.erase(0, 1);

	if (msg.empty())
		msg = "Client exited";

	Server *server = client->getServer();
	std::map<std::string, Channel *> &channels = server->getChannelMap();

	if (channels.find(chanName) == channels.end())
	{
		send(client->getFd(), "No such channel\r\n", 18, 0);
	}

for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
        std::string quitMsg = ":" + client->getNickname() + "!" + client->getUsername() +
		"@localhost QUIT :" + msg + "\r\n"; // IRC QUIT protokolüne daha uygun


        Channel* channel = it->second; // Tek değişken ile çalış
        channel->removeClient(client);
        channel->broadcast(quitMsg, client);
        break; // Bulduktan sonra çık
}


	std::cout << client->getNickname() << " exited from " << chanName << " channel" << std::endl;
}
