#include "Bender.hpp"

Bender::Bender() : name("bender"),
                    port("8080"),
                    levelOfHoliness(1),
                    connectionFd(-1),
                    state(PASS),
                    nickTries(0)
{
    #ifdef DEBUG
        std::cout << "Bender default constructor called\n";
    #endif
}

Bender::Bender(const std::string& nick, const std::string& pass,
                const char* thePort, int level) : name(nick),
                                                password(pass),
                                                port(thePort),
                                                levelOfHoliness(level),
                                                connectionFd(-1),
                                                state(PASS),
                                                nickTries(0)
{
    std::memset(&connectionPoll, 0, sizeof(connectionPoll));
    #ifdef DEBUG
        std::cout << "Bender constructor called\n";
    #endif
}

Bender::Bender(const Bender& bender) : name(bender.name),
                                    password(bender.password),
                                    port(bender.port),
                                    levelOfHoliness(bender.levelOfHoliness),
                                    connectionFd(bender.connectionFd),
                                    state(bender.state),
                                    nickTries(bender.nickTries)

{
    std::memcpy(&connectionPoll, &bender.connectionPoll, sizeof(pollfd));
    #ifdef DEBUG
        std::cout << "Bender copy constructor called\n";
    #endif
}

Bender& Bender::operator=(const Bender& other)
{
    if (this != &other)
    {
        if (connectionFd != -1)
            close(connectionFd);
        connectionFd = other.connectionFd;
        name = other.name;
        port = other.port;
        password = other.password;
        levelOfHoliness = other.levelOfHoliness;
        connectionPoll = other.connectionPoll;
        state = other.state;
        nickTries = other.nickTries;
        std::memcpy(&connectionPoll, &other.connectionPoll, sizeof(pollfd));
    }
    #ifdef DEBUG
        std::cout << "Bender copy operator called\n";
    #endif
    return (*this);
}
Bender::~Bender()
{
    if (connectionFd != -1)
    {
        close(connectionFd);
    }
    #ifdef DEBUG
        std::cout << "Bender destructor called\n";
    #endif
}

void Bender::connectToServer()
{
    addrinfo    hints;
    addrinfo*   info;
    addrinfo*   it; 
    int         err;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    err = getaddrinfo("localhost", port, &hints, &info);
    if (err != 0)
        throw (std::runtime_error("Error in connecting to the host"));
    
    for(it = info; it != NULL; it = it->ai_next)
    {
        connectionFd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (connectionFd == -1)
            continue;
        if (connect(connectionFd, it->ai_addr, it->ai_addrlen) == -1)
        {
            close(connectionFd);
            std::cout << "Error : " << strerror(errno) << '\n';
            continue;
        }
        break;
    }
    err = (it == NULL);
    freeaddrinfo(info);
    if (err)
        throw std::runtime_error("Could not connect to the server");
    fcntl(connectionFd, F_SETFL, O_NONBLOCK);
    connectionPoll.fd = connectionFd;
    connectionPoll.events = POLLIN | POLLOUT;
    connectionPoll.revents = 0;
    enqueueMsg("PASS " + password + "\r\nNICK " + name + "\r\nUSER bender 0 * :RealOg\r\n");
}

void Bender::pollEvents()
{
    int     ret;
    
    ret = poll(&connectionPoll, 1, -1);
    if (ret == -1)  
        throw (std::runtime_error(strerror(errno)));
    if (connectionPoll.revents & POLLOUT)
        dequeueMsg();
    else if (connectionPoll.revents & POLLIN)
        handleServerReply();
}

bool    endLineReceived(const std::string& message)
{
    for (std::string::const_iterator it = message.begin(); it != message.end(); ++it)
    {
        if (*it == '\n' ||
            (*it == '\r' && *(it + 1) == '\n'))
        {
            return(true);
        }
    }
    return (false);
}

void Bender::handleServerReply()
{
    std::vector<std::string> splittedReply;
    std::string              arg0;
    int                      ret;

    if ((ret = recv(connectionFd, buffer, 512, 0)) < 0)
        throw std::runtime_error("error in reading message from server");
    if (ret == 0)
        throw std::runtime_error("they got us! Server closed the connection!!");
    std::cout << "message from the server\n" << buffer;
    std::string msgFromServer(buffer);
    if (!endLineReceived(msgFromServer))
        return;
    splittedReply.reserve(5);
    split(msgFromServer, ' ', splittedReply);
    std::memset(buffer, 0, ret);
    arg0 = splittedReply.at(1);
    if (splittedReply.at(0) == ":localhost")
    {
        if (arg0 == "464")
            throw std::runtime_error("You gave me the wrong password!");
        if (arg0 == "432")
            enqueueMsg("NICK bender\r\n");
        else if (arg0 == "433")
        {
            if (nickTries == 1)
                name += "OG";
            else if (nickTries == 2)
                name += "theReal";
            else if (nickTries == 3)
                name += "OneAndOnly";
            else
                throw std::runtime_error("Cannot find a nickname! Bye bye");
            nickTries++;
            enqueueMsg("NICK " + name + "\r\n");
        }
        else if (arg0 == "001")
            std::cout << "WE ARE IN MOTHERFUCKER!!\n";
    }
}

void Bender::enqueueMsg(const std::string& message)
{
    toSend.push(message);
    connectionPoll.events |= POLLOUT;
}

void Bender::dequeueMsg()
{
    ssize_t ret;

    if (toSend.empty())
    {
        std::cout << "called with empty queue\n";
        connectionPoll.events = POLLIN;
        return;
    }
    std::string msgToSend(toSend.front());
    ret = send(connectionFd, msgToSend.data(), msgToSend.length(), 0);
    if (ret < 0)
        throw std::runtime_error("Error in sending data to the server");
    if ((size_t)ret != msgToSend.length()) //handle data partially sent
    {
        toSend.front() = toSend.front().substr(ret);
        return;
    }
    toSend.pop();
    if (toSend.empty())
    {
        connectionPoll.events = POLLIN;
    }
}