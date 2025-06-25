#include "Connection.hpp"

Connection::Connection() :  user(),
                            bytesInBuffer(0), 
                            fd(-1)
{
    connectionPoll = NULL;
    std::memset(&peeraddr, 0, sizeof(peeraddr));
    #ifdef DEBUG
        std::cout << "Connection default constructor called\n";
    #endif
}
Connection::Connection(int connectionFd, sockaddr_storage* addr) :  user(),
                                                                    bytesInBuffer(0),
                                                                    fd(connectionFd) 
{
    connectionPoll = NULL;
    std::memcpy(&peeraddr, addr, sizeof(peeraddr));
    #ifdef DEBUG
        std::cout << "Connection constructor called\n";
    #endif
}

Connection::Connection(const Connection& connection) :  user(connection.user), 
                                                        bytesInBuffer(connection.bytesInBuffer),
                                                        connectionPoll(connection.connectionPoll),
                                                        msgQueue(connection.msgQueue),
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
        msgQueue = other.msgQueue;
        connectionPoll = other.connectionPoll;
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

pollfd* Connection::getPollFd() const
{
    return (connectionPoll);
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

const std::string&  Connection::getMask() const
{
    return (user.getMask());
}

std::queue<std::string>& Connection::getQueue()
{
    return (msgQueue);
}

bool    Connection::isAuthenticated() const
{
    return (user.isAuthenticated());
}

bool    Connection::isRegistered() const
{
    return (user.isRegistered());
}

void Connection::enqueueMsg(const std::string& msg)
{
    msgQueue.push(msg);
    connectionPoll->events |= POLLOUT;
}

void Connection::setConnectionPoll(pollfd* pdf)
{
    connectionPoll = pdf;
}

void Connection::setFd(int newFd)
{
    if (fd != -1)
        close (fd);
    fd = newFd;
}

void Connection::setUser(User& newUser)
{
    user = newUser;
}

void Connection::setNickname(const std::string& nick)
{
    user.setNickname(nick);
}

void Connection::setUsername(const std::string& username)
{
    user.setUsername(username);
}

void Connection::setFullname(const std::string& fullname)
{
    user.setFullname(fullname);
}

void Connection::setMask(const std::string& mask)
{
    user.setMask(mask);
}

void Connection::setAuth(bool auth)
{
    user.setAuthenticate(auth);
}

void Connection::closeConnection()
{
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
    bytes = recv(fd, buffer + bytesInBuffer, 512 - bytesInBuffer, 0);
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
    std::memset(buffer, 0, bytesInBuffer - 1);
    bytesInBuffer = 0;
}

ssize_t Connection::dequeueMsg()
{
    ssize_t ret;

    if (msgQueue.empty()) //useless if, check if this ever happens
    {
        std::cout << "called with empty queue\n";
        connectionPoll->events = POLLIN;
        return (0);
    }
    std::string msgToSend(msgQueue.front());
    ret = send(fd, msgToSend.data(), msgToSend.length(), 0);
    if (ret < 0)
        return (ret);
    if ((size_t)ret != msgToSend.length()) //handle data partially sent
    {
        msgQueue.front() = msgQueue.front().substr(ret); //save in the front of the queue the data that was not sent
        return (1);
    }
    msgQueue.pop();
    if (msgQueue.empty())
    {
        connectionPoll->events = POLLIN;
    }
    return (1);
}