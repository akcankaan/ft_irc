#include "../CommandHandler.hpp"
#include "../../server/Server.hpp"
#include "../../channel/Channel.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>


void quit(Client *client, std::istringstream &iss)
{
    std::string dummy; // Artık tek bir kanal adı değil, quit mesajı parse edilecek
    iss >> dummy; // Çoğu zaman boş gelir

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

    std::string quitMsg = ":" + client->getNickname() + "!" + client->getUsername() +
                          "@localhost QUIT :" + msg + "\r\n";

    // Kullanıcının içinde bulunduğu tüm kanalları bul
    std::vector<Channel*> userChannels;
    for (std::map<std::string, Channel *>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        Channel* channel = it->second;
        const std::vector<Client*>& clist = channel->getClients();
        if (std::find(clist.begin(), clist.end(), client) != clist.end())
            userChannels.push_back(channel);
    }

    for (size_t i = 0; i < userChannels.size(); ++i)
    {
        Channel* channel = userChannels[i];
        std::string chanName = channel->getName();

        bool wasOperator = channel->isOperator(client->getNickname());

        // Client'ı kanaldan sil
        channel->removeClient(client);

        // QUIT mesajı
        channel->broadcast(quitMsg, client);

        // Eğer çıkan kullanıcı operator ise ve kanalda başka kullanıcı varsa
        if (wasOperator && !channel->getClients().empty())
        {
            // Son giren kullanıcıyı bul (en sondaki)
            Client* newOp = channel->getClients().back();
            channel->addOperator(newOp->getNickname());

            std::string modeMsg = ":@localhost MODE " + chanName +
                " +o " + newOp->getNickname() + "\r\n";
            channel->broadcast(modeMsg, NULL);
        }

        std::cout << client->getNickname() << " exited from " << chanName << " channel" << std::endl;
    }

    // Server'dan çıkarma işlemleri (isteğe bağlı, aşağıdaki satırı kendi sistemine göre düzenle)
    server->removeClient(client->getFd()); // Böyle bir fonksiyonun varsa kullanırsın.

}
