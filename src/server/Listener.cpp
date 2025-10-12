#include "Listener.hpp"

Listener::Listener() : hostname("127.0.0.1"), port("8080"), socketFd(-1)
{
    #ifdef DEBUG
        std::cout << "Listener default constructor called\n";
    #endif
}

Listener::Listener(const char* prt) : hostname("127.0.0.1"), port(prt), socketFd(-1)
{
    #ifdef DEBUG
        std::cout << "Listener constructor called\n";
    #endif
}

Listener::Listener(const Listener& listener) : 
        hostname(listener.hostname), port(listener.port), socketFd(listener.socketFd) 
{
    #ifdef DEBUG
        std::cout << "Listener copy constructor called\n";
    #endif
}

Listener& Listener::operator=(const Listener& listener)
{
    if (this != &listener)
    {
        hostname = listener.hostname;
        port = listener.port;
        if (socketFd != -1)
            close(socketFd);
        socketFd = listener.socketFd;
    }
    #ifdef DEBUG
        std::cout << "Listener copy assignment operator called\n";
    #endif
    return (*this);
}

Listener::~Listener()
{
    /* if (_socketFd != -1)
        close(_socketFd); */
    #ifdef DEBUG
        std::cout << "Listener destructor called\n";
    #endif
}
void            Listener::createSocket(int ai_family, int ai_socktype)
{
    int                 err;
    int                 yes = 1;
    struct addrinfo*    info;
    struct addrinfo*    it;
    struct addrinfo     hints;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = ai_family;
    hints.ai_socktype = ai_socktype;
    hints.ai_flags = AI_PASSIVE;
    err = getaddrinfo(hostname, port, &hints, &info);
    if (err != 0)
        throw std::runtime_error("Error in getting the address info");
    for (it = info; it != NULL; it = it->ai_next)
    {
        socketFd = socket(it->ai_family, it->ai_socktype | SOCK_NONBLOCK, it->ai_protocol);
        if (socketFd == -1)
            continue;
        setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        if (bind(socketFd, it->ai_addr, it->ai_addrlen) == 0)
            break;
    }
    err = (it == NULL);
    freeaddrinfo(info);
    if (err)
        throw std::runtime_error("Could not bind to any socket");
}

void            Listener::startListen(int queue) const
{
    if (socketFd == -1)
        return;
    if (listen(socketFd, queue) != 0)
        throw std::runtime_error(strerror(errno));
    #ifdef DEBUG
        std::cout << "Listening on port " << port << '\n';
    #endif
}
int             Listener::getSocketFd() const
{
    return (socketFd);
}

const char*     Listener::getPort() const
{
    return (port);
}

void            Listener::setNonBlockState(bool nonBlock)
{
    int flags = fcntl(socketFd, F_GETFL, 0);

    if (nonBlock)
        fcntl(socketFd, F_SETFL, flags | O_NONBLOCK);
    else
        fcntl(socketFd, F_SETFL, flags & (~O_NONBLOCK));
}

void            Listener::setSocketFd(int fd)
{
    socketFd = fd;
}

bool    Listener::isOpen() const
{
    return (socketFd != -1);
}

Connection      Listener::acceptConnection() 
{
    int                     newFd;
    struct sockaddr_storage s;
    socklen_t               addrlen;

    addrlen = sizeof(s);
    newFd = accept(socketFd, (struct sockaddr*)&s, &addrlen);
    if (newFd == -1) //cannot accept more connections from this socket, stop listening
    {
        close(socketFd);
        socketFd = -1;
        throw(std::runtime_error("Reached max socket capacity"));
    }
    fcntl(newFd, F_SETFL, O_NONBLOCK);
    return (Connection(newFd, &s));
}