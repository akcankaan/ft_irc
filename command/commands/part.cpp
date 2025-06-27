#include "../CommandHandler.hpp"
#include "../../server/Server.hpp"
#include "../../channel/Channel.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>

void part(Client *client, std::istringstream &iss)
{
	std::string chanName;
	iss >> chanName;

	std::string reason;
	std::getline(iss, reason);
	if (client->shouldDisconnect())
		return;
	if (!reason.empty() && reason[0] == ' ')
		reason.erase(0, 1);
	if (!reason.empty() && reason[0] == ':')
		reason.erase(0, 1);

	Server *server = client->getServer();
	std::map<std::string, Channel *> &channels = server->getChannelMap();

	if (channels.find(chanName) == channels.end())
	{
		std::string errorMsg = "403 " + chanName + " :No such channel\r\n";
		send(client->getFd(), errorMsg.c_str(), errorMsg.length(), 0);
		return;
	}

	Channel *channel = channels[chanName];

	if (!channel->isClientInChannel(client))
	{
		std::string errorMsg = "442 " + chanName + " :You're not on that channel\r\n";
		send(client->getFd(), errorMsg.c_str(), errorMsg.length(), 0);
		return;
	}

	bool wasOperator = channel->isOperator(client->getNickname());

	std::string partMsg = ":" + client->getNickname() + "!" + client->getUsername() +
		"@localhost PART " + chanName;

	if (!reason.empty())
		partMsg += " :" + reason;
	partMsg += "\r\n";

	channel->broadcast(partMsg, client);

	send(client->getFd(), partMsg.c_str(), partMsg.length(), 0);

	channel->removeClient(client);

	if (wasOperator && !channel->getClients().empty())
	{
		channel->removeOperator(client->getNickname());

		bool hasOp = false;
		for (size_t i = 0; i < channel->getClients().size(); ++i)
		{
			if (channel->isOperator(channel->getClients()[i]->getNickname()))
			{
				hasOp = true;
				break;
			}
		}

		if (!hasOp)
		{
			Client *newOp = channel->getClients()[0];
			channel->addOperator(newOp->getNickname());

			std::string modeMsg = ":@localhost MODE " + chanName +
				" +o " + newOp->getNickname() + "\r\n";
			channel->broadcast(modeMsg, NULL);
		}
	}

	if (channel->getClients().empty()) {
    	delete channel;
    	channels.erase(chanName);
	}
}
