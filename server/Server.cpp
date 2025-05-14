#include "Server.hpp"

Server::Server(int port, const std::string &password)
    : _port(port), _password(password), _server_fd(-1)
{
    setupSocket();
}

Server::~Server()
{
    close(_server_fd);
    for (size_t i = 0; i < _poll_fds.size(); ++i)
        close(_poll_fds[i].fd);

    for (std::map<std::string, Channel *>::iterator it = _channels.begin(); it != _channels.end(); ++it)
        delete it->second;
    _channels.clear();

    for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        delete it->second;
    _clients.clear();
}

void Server::setupSocket()
{
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd < 0)
        throw std::runtime_error("Socket oluşturulamadı");

    fcntl(_server_fd, F_SETFL, O_NONBLOCK);

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(_port);

    if (bind(_server_fd, (sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        throw std::runtime_error("Bind başarısız");

    if (listen(_server_fd, MAX_CLIENTS) < 0)
        throw std::runtime_error("Listen başarısız");

    pollfd pfd;
    pfd.fd = _server_fd;
    pfd.events = POLLIN;
    _poll_fds.push_back(pfd);

    std::cout << "Server " << _port << " portunda dinliyor..." << std::endl;
}

void Server::acceptNewClient()
{
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(_server_fd, (sockaddr *)&client_addr, &client_len);
    if (client_fd < 0)
        return;

    fcntl(client_fd, F_SETFL, O_NONBLOCK);

    pollfd pfd;
    pfd.fd = client_fd;
    pfd.events = POLLIN;
    _poll_fds.push_back(pfd);

    _clients[client_fd] = new Client(client_fd);

    std::cout << "Yeni client bağlandı: " << client_fd << std::endl;
}

void Server::handleClientMessage(int fd)
{
    char buffer[512];
    int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0)
    {
        std::cout << "Client ayrıldı: " << fd << std::endl;
        removeClient(fd);
        return;
    }

    buffer[bytes] = '\0';
    std::cout << "Client " << fd << " mesaj: " << buffer;

    std::string msg(buffer);

    // Basit bir JOIN komutu kontrolü örneği:
    if (msg.find("JOIN ") == 0)
    {
        std::string channelName = msg.substr(5);
        if (!channelName.empty() && channelName[channelName.size() - 1] == '\n')
            channelName.erase(channelName.size() - 1); // \n sil
        handleJoinCommand(fd, channelName);
    }
}

void Server::removeClient(int fd)
{
    close(fd);
    delete _clients[fd];
    _clients.erase(fd);

    for (std::vector<pollfd>::iterator it = _poll_fds.begin(); it != _poll_fds.end(); ++it)
    {
        if (it->fd == fd)
        {
            _poll_fds.erase(it);
            break;
        }
    }
}

void Server::run()
{
    while (true)
    {
        if (poll(&_poll_fds[0], _poll_fds.size(), -1) < 0)
            throw std::runtime_error("poll() başarısız");

        for (size_t i = 0; i < _poll_fds.size(); ++i)
        {
            if (_poll_fds[i].revents & POLLIN)
            {
                if (_poll_fds[i].fd == _server_fd)
                    acceptNewClient();
                else
                    handleClientMessage(_poll_fds[i].fd);
            }
        }
    }
}

void Server::handleJoinCommand(int fd, const std::string &channelName)
{
    if (channelName.empty() || channelName[0] != '#')
    {
        std::cout << "Geçersiz kanal ismi: " << channelName << std::endl;
        return;
    }

    if (_channels.find(channelName) == _channels.end())
    {
        _channels[channelName] = new Channel(channelName);
        std::cout << "Yeni kanal oluşturuldu: " << channelName << std::endl;
    }

    Channel *channel = _channels[channelName];

    if (!channel->hasMember(fd))
    {
        channel->addMember(fd);

        if (channel->getMembers().size() == 1)
            channel->addOperator(fd);

        std::cout << "Client " << fd << " -> " << channelName << " kanalına katıldı" << std::endl;
    }
    else
    {
        std::cout << "Client " << fd << " zaten " << channelName << " kanalında." << std::endl;
    }
}