#ifndef LISTENER_HPP
#define LISTENER_HPP

#include "Connection.hpp"

class Listener
{
private:
    const char* hostname;
    const char* port;
    int         socketFd;

public:
    Listener();
    Listener(const char* prt);
    Listener(const Listener& listener);
    Listener& operator=(const Listener& listener);
    ~Listener();


    void            createSocket(int ai_family = AF_UNSPEC, 
                                 int ai_socktype = SOCK_STREAM);
    void            startListen(int queue) const;
    int             getSocketFd() const;
    const char*     getPort() const;
    void            setNonBlockState(bool nonBlock);
    void            setSocketFd(int fd);

    Connection      acceptConnection();
};

#endif // LISTENER.HPP