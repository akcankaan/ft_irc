#include <iostream>
#include <cstdlib> // atoi
#include "server/Server.hpp"

bool isValidPort(const std::string &str)
{
    for (size_t i = 0; i < str.size(); ++i)
        if (!isdigit(str[i]))
            return false;
    int port = std::atoi(str.c_str());
    return port > 0 && port <= 65535;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Kullanım: ./ircserv <port> <password>" << std::endl;
        return 1;
    }

    std::string portStr = argv[1];
    std::string password = argv[2];

    if (!isValidPort(portStr))
    {
        std::cerr << "Hata: Geçersiz port numarası. 1-65535 aralığında bir değer girin." << std::endl;
        return 1;
    }

    int port = std::atoi(portStr.c_str());

    try
    {
        Server server(port, password);
        server.run(); // Ana sunucu döngüsü
    }
    catch (const std::exception &e)
    {
        std::cerr << "Sunucu hatası: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
