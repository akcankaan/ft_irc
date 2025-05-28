#include "../CommandHandler.hpp"
#include "../../server/Server.hpp"
#include "../../channel/Channel.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <cstdlib>
#include <string.h>

void quit(Client *client, std::istringstream &iss)
{
	std::string chanName;
	iss >> chanName;

	std::string msg;
	std::getline(iss, msg); // tüm kalan mesajı al

	if (!msg.empty() && msg[0] == ' ')
		msg.erase(0, 1); // baştaki boşluğu sil

	if (!msg.empty() && msg[0] == ':')
		msg.erase(0, 1); // baştaki ':' varsa sil

	if (msg.empty())
		msg = "Client exited";

	Server *server = client->getServer();
	std::map<std::string, Channel *> &channels = server->getChannelMap();

	if (channels.find(chanName) == channels.end())
	{
		send(client->getFd(), "No such channel\r\n", 18, 0);
		return;
	}

	Channel *channel = channels[chanName];
	channel->removeClient(client);

	std::string quitMsg = ":" + client->getNickname() + "!" + client->getUsername() +
		"@localhost QUIT " + chanName + " :" + msg + "\r\n";

	channel->broadcast(quitMsg, client);

	std::cout << client->getNickname() << " exited from " << chanName << " channel" << std::endl;
}
