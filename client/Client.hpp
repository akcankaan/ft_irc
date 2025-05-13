#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client {
    private:
        int _fd;
        std::string _nickname;
        std::string _username;
        std::string _realname;
        std::string _hostname;
        bool _authenticated;
        std::string _buffer;
    public:
        Client(int fd);

        int getFd() const;
        const std::string& getNickname() const;
        const std::string& getUsername() const;
        const std::string& getRealname() const;
        bool isAuthenticated() const;
        std::string& getBuffer();

        void setNickname(const std::string& nick);
        void setUsername(const std::string& user);
        void setRealname(const std::string& real);
        void setAuthenticated(bool status);
};

#endif