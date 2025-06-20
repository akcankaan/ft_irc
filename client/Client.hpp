#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>

class Server;

class Client {
    private:
        int _fd;
        std::string _nickname;
        std::string _username;
        std::string _realname;
        std::string _hostname;
        std::string _buffer;
        bool    _isAuthenticated;
        bool    _hasGivenPassword;

        Server* _server;

        bool _shouldDisconnect;


    public:
        Client(int fd);
        ~Client();

        int getFd() const;
        const std::string &getNickname() const;
        const std::string &getUsername() const;
        const std::string &getBuffer() const;
        bool    isAuthenticated() const;

        void    setNickname(const std::string &nick);
        void    setUsername(const std::string &user);
        void    appendBuffer(const std::string &data);
        void    clearBuffer();
        void    authenticate();
        bool hasGivenPassword() const;
        void markPasswordGiven();

        void setServer(Server *server);
        Server *getServer() const;

        
        void setDisconnected(bool value);
        bool shouldDisconnect() const;
};

#endif
