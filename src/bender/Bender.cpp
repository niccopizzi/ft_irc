#include "Bender.hpp"

Bender::Bender(const std::string& pass, const std::string& theName, const char* thePort) : name(theName),
                                                                password(pass),
                                                                port(thePort),
                                                                connectionFd(-1),
                                                                nickTries(0)
{
    #ifdef DEBUG
        std::cout << "Bender constructor called\n";
    #endif
}


Bender::Bender(const Bender& bender) : name(bender.name),
                                    password(bender.password),
                                    port(bender.port),
                                    connectionFd(bender.connectionFd),
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
        connectionPoll = other.connectionPoll;
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

bool    endLineReceived(std::string& message)
{
    for (std::string::iterator it = message.begin(); it != message.end(); ++it)
    {
        if (*it == '\n')
        {
            message.erase(it);
            return (true);
        }
        if (*it == '\r' && *(it + 1) == '\n')
        {
            message.erase(it + 1);
            message.erase(it);
            return (true);
        }
    }
    return (false);
}

void Bender::handleServerReply()
{
    std::vector<std::string> splittedReply;
    std::string              arg0;
    int                      ret;
    char                     msgBuf[513];

    if ((ret = recv(connectionFd, msgBuf, 512, 0)) < 0)
        throw std::runtime_error("error in reading message from server");
    if (ret == 0)
        throw std::runtime_error("they got us! Server closed the connection!!");
    msgBuf[ret] = 0;
    std::cout << "message from the server\n" << msgBuf;
    std::string msgFromServer(msgBuf);
    if (!endLineReceived(msgFromServer))
        return;
    splittedReply.reserve(5);
    split(msgFromServer, ' ', splittedReply);
    std::memset(buffer, 0, ret);
    arg0 = splittedReply.at(1);
    if (splittedReply.at(0) == ":localhost")
    {
        handleServerMessage(splittedReply);
    }
    else
        handleUserAction(splittedReply);
}

void Bender::storeChannel(const std::vector<std::string>& args)
{
    channels.insert(args.at(2));
}

void Bender::storeChannelUsers(const std::vector<std::string>& args)
{
    std::set<std::string> users;

    for (std::vector<std::string>::const_iterator it = args.begin() + 3; it != args.end(); ++it)
    {
        users.insert(*it);
    }
    channelToUsers.insert(std::pair<std::string, std::set<std::string> >(args.at(3), users));
    
}

void Bender::handleServerMessage(const std::vector<std::string>& msg)
{
    const std::string& num = msg.at(1);

    if (num == "464")
            throw std::runtime_error("You gave me the wrong password!");
    if (num == "432") //erroneous nickname
        enqueueMsg("NICK bender\r\n");
    else if (num == "433") //nickname already in use
    {
        if (nickTries == 0)
            name += "OG"; 
        else if (nickTries == 1)
            name += "theReal";
        else if (nickTries == 2)
            name += "OneAndOnly";
        else if (nickTries == 3)
            name += "SnoopDogg";
        else
            throw std::runtime_error("Cannot find a nickname! Bye bye");
        nickTries++;
        enqueueMsg("NICK " + name + "\r\n");
    }
    else if (num == "001")
        enqueueMsg("LIST\r\n");
    else if (num == "322")
        storeChannel(msg);
    else if (num == "352")
        storeChannelUsers(msg);
}

void Bender::handleUserAction(const std::vector<std::string>& msg)
{
    const std::string& action = msg.at(1);

    if (action == "PRIVMSG")
    {
        const std::string& target = msg.at(2);
        if (target == name)
            handlePrivateMsg(msg);
        else
            handleChannelMsg(msg);
    }
    else if (action == "INVITE")
        enqueueMsg("JOIN " + msg.at(3) + "\r\n");
    else if (action == "MODE")
    {
        handleModeChange(msg);
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
