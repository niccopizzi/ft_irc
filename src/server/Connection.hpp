#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "../user/User.hpp"
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
#include <queue>


#define BUFFER_FULL         -3
#define READ_ERROR          -2
#define CONNECTION_CLOSED   -1
#define ENDLINE_RECEIVED     0
#define NO_END               1


typedef long    connectionID;

class Connection
{
private:
    sockaddr_storage            peeraddr;
    User                        user;
    std::string                 clientMessage;
    size_t                      bytesInBuffer;
    pollfd*                     connectionPoll;
    std::queue<std::string>     msgQueue;
    int                         fd;
    connectionID                id; //unique id that identifies the connection
    
    public:
    Connection();
    Connection(int connectionFd, sockaddr_storage* addr);
    Connection(int connectionFd, connectionID conId, sockaddr_storage* addr);
    Connection(const Connection& connection);
    Connection& operator=(const Connection& other);
    ~Connection();
    
    char    buffer[513];

    std::queue<std::string>&    getQueue();
    pollfd*             getPollFd() const;
    int                 getFd() const;
    connectionID        getConnectionId() const;
    const User&         getUser() const;
    const std::string&  getUsername() const;
    const std::string&  getNickname() const;
    const std::string&  getFullname() const;
    const std::string&  getMask()   const;
    void                setConnectionPoll(pollfd* pdf);
    void                setId(connectionID newId);
    void                setFd(int fd);
    void                setUser(User& user);
    void                setNickname(const std::string& nick);
    void                setUsername(const std::string& username);
    void                setFullname(const std::string& fullname);
    void                setMask(const std::string& mask);
    void                enqueueMsg(const std::string& msg);
    ssize_t             dequeueMsg();
    void                setAuth(bool auth);
    void                closeConnection();
    void                clearBuffer();
    //void                sendMessage(const std::string& msg) const;
    bool                isAuthenticated(void) const;
    bool                isRegistered(void) const;
    int                 handleClientMsg();
};

#endif