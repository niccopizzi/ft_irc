#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "../Users/User.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <poll.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <ctime>


#define BUFFER_FULL         -3
#define READ_ERROR          -2
#define CONNECTION_CLOSED   -1
#define ENDLINE_RECEIVED     0
#define NO_END               1


class Connection
{
private:
    struct sockaddr_storage     peeraddr;
    User                        user;
    std::string                 clientMessage;
    size_t                      bytesInBuffer;
    
    public:
    Connection();
    Connection(int connectionFd, struct sockaddr_storage* addr);
    Connection(const Connection& connection);
    Connection& operator=(const Connection& other);
    ~Connection();
    
    int     fd;
    char    buffer[513];


    int                 getFd() const;
    const User&         getUser() const;
    const std::string&  getUsername() const;
    const std::string&  getNickname() const;
    const std::string&  getFullname() const;
    void                setFd(int fd);
    void                setUser(User& user);
    void                setNickname(const std::string& nick);
    void                setUsername(const std::string& username);
    void                setFullname(const std::string& fullname);
    void                setAuth(bool auth);
    void                closeConnection();
    void                clearBuffer();
    void                sendMessage(const std::string& msg);
    bool                isAuthenticated(void) const;
    bool                isRegistered(void) const;
    int                 handleClientMsg();
};

#endif