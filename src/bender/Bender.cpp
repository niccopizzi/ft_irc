#include "Bender.hpp"

Bender::Bender(const std::string& pass, 
                const std::string& theName, 
                const char* thePort) :  name(theName),
                                        password(pass),
                                        port(thePort),
                                        connectionFd(-1),
                                        nickTries(0)
{
}

Bender::Bender(const Bender& bender) :  name(bender.name),
                                        password(bender.password),
                                        port(bender.port),
                                        connectionFd(bender.connectionFd),
                                        nickTries(bender.nickTries)

{
    std::memcpy(&connectionPoll, &bender.connectionPoll, sizeof(pollfd));
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
    return (*this);
}

Bender::~Bender()
{
    if (connectionFd != -1)
    {
        close(connectionFd);
    }
    for (std::map<std::string, ChannelInfo*>::iterator it = channelsJoined.begin();
            it != channelsJoined.end(); ++it)
    {
        delete it->second;
    }
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
    enqueueMsg("PASS " + password + "\r\nNICK " + name + 
                "\r\nUSER bender 0 * :RealOg\r\n");
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
    std::stringstream ss(msgBuf);
    if (!ss.str().find('\n'))
        return;
    splittedReply.reserve(5);
    std::string line;
    while (std::getline(ss, line))
    {
        if(*(line.end() - 1) == '\r')
            line.erase(line.end() - 1);
        split(line, ' ', splittedReply);
        arg0 = splittedReply.at(1);
        if (splittedReply.at(0) == ":localhost")
        {
            handleServerMessage(splittedReply);
        }
        else
            handleUserAction(splittedReply);
        line.clear();
        splittedReply.clear();
    }
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
        changeNick();
    }
    else if (num == "001") //welcome reply from server
        enqueueMsg("LIST\r\n"); //ask list of channels 
    else if (num == "322") //reply to LIST command
        storeChannel(msg);
    //else if (num == "353") //reply to JOIN command with list of users in the channel
    else if (num == "352") //reply to WHO command
        storeChannelUsers(msg);
}

void Bender::storeChannel(const std::vector<std::string>& args)
{
    channelsOnServer.insert(args.at(3));
}

void Bender::storeChannelUsers(const std::vector<std::string>& args)
{
    std::set<std::string>   users;
    ChannelInfo*            newChannelInfo;

    for (std::vector<std::string>::const_iterator it = args.begin() + 4; 
        it != args.end(); ++it)
    {
        users.insert(*it);
    }
    newChannelInfo = new ChannelInfo(users);
    channelsJoined.insert(std::make_pair(args.at(3), newChannelInfo));
}

void Bender::changeNick()
{
    switch (nickTries)
    {
    case 0:
        name += "OG";
        break;
    case 1:
        name += "theReal";
        break;
    case 2:
        name += "OneAndOnly";
        break;
    case 3:
        name += "SnoopDogg";
        break;
    default:
        throw std::runtime_error("Cannot find a nickname! Bye bye");
    }
    nickTries++;
    enqueueMsg("NICK " + name + "\r\n");
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
    else if (action == "INVITE" && msg.at(2) == name)
        enqueueMsg("JOIN " + msg.at(3) + "\r\n");
    else if (action == "MODE")
    {
        handleModeChange(msg);
    }
    else if (action == "JOIN")
        handleJoin(msg);
}

void Bender::handleJoin(const std::vector<std::string>& msg)
{
    std::string         nickOfJoiner(msg.at(0));
    std::string         channel(msg.at(2));
    size_t              nicklen;

    if (channel.at(0) == ':')
        channel.erase(0, 1);
    if (nickOfJoiner.at(0) == ':')
        nickOfJoiner.erase(0, 1);
    nicklen = nickOfJoiner.find('!');
    if (nicklen != std::string::npos)
        nickOfJoiner = nickOfJoiner.substr(0, nicklen);
    if (nickOfJoiner == name) //reply Bender joininig a channel, ask who are the users
    {
        enqueueMsg("WHO " + channel + "\r\n");
    }
    else //a new person joined the channel, store the member in the channel info
    {
        ChannelInfo* channelToUpdate = channelsJoined.find(channel)->second;
        channelToUpdate->addMember(nickOfJoiner);
    }
}

void Bender::handleNickChange(const std::vector<std::string>& msg)
{
    const std::string   nickOfChanger(msg.at(0).substr(msg.at(0).find('!')));
    const std::string   newNick(msg.at(2).substr(1));

    for(std::map<std::string, ChannelInfo*>::iterator it = channelsJoined.begin(); 
            it != channelsJoined.end(); ++it)
    {
        it->second->updateMemberNick(nickOfChanger, newNick);
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

void Bender::addChannelToOperatedSet(const std::string& chanName)
{
    channelsOperated.insert(chanName);
}