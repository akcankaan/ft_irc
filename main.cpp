#include <iostream>
#include <cstdlib>    // std::atoi
#include "../server/Server.hpp" // Server sınıfı tanımı

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Kullanım: ./ircserv <port> <password>" << std::endl;
        return 1;
    }

    int port = std::atoi(argv[1]);
    if (port <= 0 || port > 65535)
    {
        std::cerr << "Geçersiz port numarası. (1 - 65535 arası olmalı)" << std::endl;
        return 1;
    }

    std::string password = argv[2];

    try
    {
        Server server(port, password); // Sunucuyu başlat
        server.run();                  // Sunucuyu döngüye sok
    }
    catch (const std::exception &e)
    {
        std::cerr << "Hata: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
