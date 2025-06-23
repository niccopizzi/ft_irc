#include "Connection.hpp"

Connection::Connection() :  user(),
                            bytesInBuffer(0), 
                            fd(-1)
{
    std::memset(&peeraddr, 0, sizeof(peeraddr));
    #ifdef DEBUG
        std::cout << "Connection default constructor called\n";
    #endif
}
Connection::Connection(int connectionFd, struct sockaddr_storage* addr) :   user(),
                                                                            bytesInBuffer(0),
                                                                            fd(connectionFd) 
{
    std::memcpy(&peeraddr, addr, sizeof(peeraddr));
    #ifdef DEBUG
        std::cout << "Connection constructor called\n";
    #endif
}

Connection::Connection(const Connection& connection) :  user(connection.user), 
                                                        bytesInBuffer(0),
                                                        fd(connection.fd) 
{
    std::memcpy(buffer, connection.buffer, connection.bytesInBuffer);
    std::memcpy(&peeraddr, &connection.peeraddr, sizeof(peeraddr));
    #ifdef DEBUG
        std::cout << "Connection copy constructor called\n";
    #endif
}

Connection& Connection::operator=(const Connection& other)
{
    if (this != &other)
    {
        if (fd != -1)
            close(fd);
        fd = other.fd;
        user = other.user;
        bytesInBuffer = other.bytesInBuffer;
        std::memcpy(buffer, other.buffer, other.bytesInBuffer);
        std::memcpy(&peeraddr, &other.peeraddr, sizeof(peeraddr));
    }
    #ifdef DEBUG
        std::cout << "Connection copy operator called\n";
    #endif
    return (*this);
}

Connection::~Connection()
{
    #ifdef DEBUG
        std::cout << "Connection destructor called\n";
    #endif
}

int Connection::getFd() const
{
    return (fd);
}

const User& Connection::getUser() const
{
    return (user);
}

const std::string& Connection::getUsername() const
{
    return (user.getUsername());
}
const std::string& Connection::getNickname() const
{
    return (user.getNickname());
}

const std::string& Connection::getFullname() const
{
    return (user.getFullname());
}

bool    Connection::isAuthenticated() const
{
    return (user.isAuthenticated());
}

bool    Connection::isRegistered() const
{
    return (user.isRegistered());
}

void Connection::setFd(int fd)
{
    if (this->fd != -1)
        close (this->fd);
    this->fd = fd;
}

void Connection::setUser(User& user)
{
    this->user = user;
}

void Connection::setNickname(const std::string& nick)
{
    this->user.setNickname(nick);
}

void Connection::setUsername(const std::string& username)
{
    this->user.setUsername(username);
}

void Connection::setFullname(const std::string& fullname)
{
    this->user.setFullname(fullname);
}

void Connection::setAuth(bool auth)
{
    user.setAuthenticate(auth);
}

void Connection::closeConnection()
{
    user.setNickname("");
    user.setUsername("");
    user.setAuthenticate(false);
    if (fd != -1)
        close(fd);
    fd = -1;
    bytesInBuffer = 0;
    std::memset(buffer, 0, sizeof(buffer));
    std::memset(&peeraddr, 0, sizeof(peeraddr));
}

bool    endLineReceived(char* buffer, size_t len)
{
    for (size_t i = 0; i < len ; ++i)
    {
        if (buffer[i] == '\n' ||
            (buffer[i] == '\r' && buffer[i + 1] == '\n'))
        {
            return(true);
        }
    }
    return (false);
}

int Connection::handleClientMsg()
{
    ssize_t  bytes;

    if (bytesInBuffer >= 510)
         return (BUFFER_FULL);
    bytes = read(fd, buffer + bytesInBuffer, 512 - bytesInBuffer);
    if (bytes == -1)
        return (READ_ERROR);
    if (bytes == 0)
        return (CONNECTION_CLOSED);
    bytesInBuffer += bytes;
    buffer[bytesInBuffer] = 0;
    if (endLineReceived(buffer, bytesInBuffer))
        return (ENDLINE_RECEIVED);
    return (NO_END);
}

void Connection::clearBuffer()
{
    std::memset(buffer, 0, bytesInBuffer);
    bytesInBuffer = 0;
}

void Connection::sendMessage(const std::string& message)
{
    send(fd, message.data(), message.length(), 0);
}