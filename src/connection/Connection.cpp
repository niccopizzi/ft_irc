#include "Connection.hpp"

Connection::Connection() :  user(),
                            bytesInBuffer(0), 
                            fd(-1),
                            id(0)
{
    connectionPoll = NULL;
    std::memset(&peeraddr, 0, sizeof(peeraddr));
}

Connection::Connection(int connectionFd, sockaddr_storage* addr) : user(),
                                                                    bytesInBuffer(0),
                                                                    fd(connectionFd),
                                                                    id(0)
{
    connectionPoll = NULL;
    timeOfLastInteraction = std::time(NULL);
    std::memcpy(&peeraddr, addr, sizeof(peeraddr));
}

Connection::Connection(int connectionFd, connectionID conId, sockaddr_storage* addr) :  user(),
                                                                                        bytesInBuffer(0),
                                                                                        fd(connectionFd),
                                                                                        id(conId) 
{
    connectionPoll = NULL;
    timeOfLastInteraction = std::time(NULL);
    std::memcpy(&peeraddr, addr, sizeof(peeraddr));
}

Connection::Connection(const Connection& connection) :  user(connection.user), 
                                                        bytesInBuffer(connection.bytesInBuffer),
                                                        connectionPoll(connection.connectionPoll),
                                                        msgQueue(connection.msgQueue),
                                                        fd(connection.fd), 
                                                        id(connection.id)
{
    #ifdef LOG
        logger = connection.logger;
    #endif
    this->timeOfLastInteraction = connection.timeOfLastInteraction;
    std::memcpy(buffer, connection.buffer, connection.bytesInBuffer);
    std::memcpy(&peeraddr, &connection.peeraddr, sizeof(peeraddr));
}

Connection& Connection::operator=(const Connection& other)
{
    if (this != &other)
    {
        if (fd != -1)
            close(fd);
        fd = other.fd;
        id = other.id;
        user = other.user;
        bytesInBuffer = other.bytesInBuffer;
        msgQueue = other.msgQueue;
        connectionPoll = other.connectionPoll;
        timeOfLastInteraction = other.timeOfLastInteraction;
        #ifdef LOG
            logger = other.logger;
        #endif
        std::memcpy(buffer, other.buffer, other.bytesInBuffer);
        std::memcpy(&peeraddr, &other.peeraddr, sizeof(peeraddr));
    }
    return (*this);
}

Connection::~Connection()
{
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

connectionID  Connection::getConnectionId() const
{
    return (id);
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

time_t Connection::getTimeOfLastInteraction() const
{
    return (timeOfLastInteraction);
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

void Connection::setId(connectionID newId)
{
    if(id == 0)
        id = newId;
}

//set the last interaction to now
void Connection::updateTimeOfLastInteraction()
{
    timeOfLastInteraction = std::time(NULL);
}

#ifdef LOG
void Connection::setLogger(Logger* logger)
{
    this->logger = logger; 
}
#endif


void Connection::closeConnection()
{
    if (fd != -1)
        close(fd);
    fd = -1;
    bytesInBuffer = 0;
    id = 0;
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

//Telnet sends a sequence of 5 bytes to signal a Ctrl+c received from the user
//This functions is to check if the sequence has been received and in case close
//the connection
bool    exitSequenceReceived(char* buffer, size_t len)  
{
    static unsigned char     sequence[5] = {0xff, 0xf4, 0xff, 0xfd, 0x06};

    if (len < 5)
        return (false);
    for (int i = 0; i < 5; ++i)
    {
        if (static_cast<unsigned char>(buffer[i]) != sequence[i])
            return (false);
    }
    return (true);
}

int Connection::handleClientMsg()
{
    ssize_t  bytes;

    if (bytesInBuffer >= 510)
    {
        clearBuffer();
        return (BUFFER_FULL);
    }
    bytes = recv(fd, buffer + bytesInBuffer, 512 - bytesInBuffer, 0); //silently truncate messages that exceed the 512 bytes limit
    if (bytes == -1)
        return (READ_ERROR);
    if (bytes == 0 || exitSequenceReceived(buffer, bytesInBuffer)) //handle EOF and Ctrl+c sequence 
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
    #ifdef LOG
        logger->logMessage("sent message", msgToSend.substr(0, msgToSend.find('\n') - 1), id);
    #endif
    ret = send(fd, msgToSend.data(), msgToSend.length(), 0);
    if (ret < 0)
        return (ret);
    if (msgToSend.find("ERROR :") != std::string::npos) //means that the fatal error message has been sent and the connection can be closed
        return (ERROR_NOTIFIED);
    if ((size_t)ret != msgToSend.length()) //handle data partially sent
    {
        msgQueue.front() = msgQueue.front().substr(ret); //save in the front of the queue the data that was not sent
        #ifdef LOG
            logger->logMessage("message sent partially - enqueued the rest", msgQueue.front(), id);
        #endif
        return (1);
    }
    msgQueue.pop();
    if (msgQueue.empty())
    {
        connectionPoll->events = POLLIN;
    }
    return (1);
}