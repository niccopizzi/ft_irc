#include "Server.hpp"

Server::Server() :  listener(), 
                    password("")
{
    polls.reserve(50);
    #ifdef DEBUG
        std::cout << "Default Server constructor called\n";
    #endif
}

Server::Server(const char* port, const char* password) :    listener(port), 
                                                            password(password)
{
    polls.reserve(50);
    #ifdef DEBUG
        std::cout << "Server constructor called\n";
    #endif
}

Server::Server(const Server& server) :  listener(server.listener), 
                                        password(server.password),
                                        polls(server.polls), 
                                        connections(server.connections),
                                        channels(server.channels),
                                        fdToConnection(server.fdToConnection),
                                        nickToConnection(server.nickToConnection)
{
    #ifdef DEBUG
        std::cout << "Server copy constructor called\n";
    #endif
}

Server& Server::operator=(const Server& other)
{
    if (this != &other)
    {
        listener = other.listener;
        password = other.password;
        polls = other.polls;
        connections = other.connections;
        channels = other.channels;
        fdToConnection = other.fdToConnection;
        nickToConnection = other.nickToConnection;
    }
    #ifdef DEBUG
        std::cout << "Server copy operator called\n";
    #endif
    return (*this);
}

Server::~Server()
{
    for (std::vector<struct pollfd>::iterator it = polls.begin();
            it != polls.end(); ++it)
    {
        if (it->fd != -1)
        {
            close(it->fd);
            it->fd = -1;
        }
    }

    #ifdef DEBUG
        std::cout << "Server destructor called\n";
    #endif
}

const Listener& Server::getListener() const
{
    return (listener);
}

const std::list<Connection>& Server::getConnections() const
{
    return (connections);
}

const std::vector<pollfd>& Server::getPolls() const
{
    return (polls);
}

const std::map<const std::string&, Connection&>& Server::getNicksMap() const
{
    return (nickToConnection);
}

const std::map<int, Connection&>&  Server::getFdMap() const
{
    return (fdToConnection);
}

void Server::openPort()
{
    pollfd listenerpoll;
    
    listener.createSocket(AF_UNSPEC, SOCK_STREAM);
    listener.startListen(SOMAXCONN);
    listenerpoll.fd = listener.getSocketFd();
    listenerpoll.events = POLLIN;
    listenerpoll.revents = 0;
    polls.push_back(listenerpoll);
}

void Server::printserver() const
{
    std::cout << "Printing server ---\n";
    
    std::cout << "CONNECTIONS\n";
    int index = 0;
    for (std::list<Connection>::const_iterator it = connections.begin();
        it != connections.end(); ++it)
    {
        std::cout << "\tConnection number : " << index <<'\n';
        std::cout << "\tConnection address : " << &(*it) << '\n';
        std::cout << "\tfd                -> " << it->getFd() << '\n';
        std::cout << "\tNickName          -> " << it->getNickname() << '\n';
        //std::cout << "\tNickName address  -> " << &it->getNickname() << '\n';
        std::cout << "\tUsername          -> " << it->getUsername() << '\n';
        std::cout << "\tRegistered        -> " << (it->isRegistered() ? "Yes" : "No") << '\n';
        index++;
    }
    index = 0;
    std::cout << "\nCHANNELS\n";
    for (std::map<std::string, Channel>::const_iterator ct = channels.begin(); ct != channels.end(); ++ct)
    {
        std::cout << "\tChannel number : " << index <<'\n';
        std::cout << "\tChannel name   -> " << ct->first <<'\n';
        std::cout << "\tChannle key    -> " << ct->second.getKey() << '\n';
        std::cout << "\tChannel members\n" <<'\n';
        int memnum = 0;
        const std::map<const std::string&, Connection&>& members = ct->second.getMembers();
        for (std::map<const std::string&, Connection&>::const_iterator member = members.begin();
            member != members.end(); ++member)
        {
            std::cout << "\t\tMember number    : " << memnum << '\n';
            std::cout << "\t\tMember address   : " << &(member->second) << '\n';
            std::cout << "\t\tMember fd       -> " << member->second.getFd() << '\n';  
            std::cout << "\t\tMember nickname -> " << member->first << '\n';
            std::cout << "\t\tMember username -> " << member->second.getUsername() << '\n';
            std::cout << "\t\tRegistered      -> " << (member->second.isRegistered() ? "yes" : "no") << '\n';
            memnum++;
        }
    }/* 
    std::cout << "\nFDTOCONNECTION\n";
    for (std::map<int, Connection&>::const_iterator it = fdToConnection.begin(); it != fdToConnection.end(); ++it)
    {
        std::cout << "\tFd                  -> " << it->first << '\n';
        std::cout << "\tNickname            -> " << it->second.getNickname() << '\n';
        std::cout << "\tUsername            -> " << it->second.getUsername() << '\n';
        std::cout << "\tConnection address  ->" << &it->second << '\n';
    }
    std::cout << "\nNICKTOCONNECTION\n";
    for (std::map<const std::string&, Connection&>::const_iterator it = nickToConnection.begin(); it != nickToConnection.end(); ++it)
    {
        std::cout << "\tFd                  -> " << it->second.fd << '\n';
        std::cout << "\tNickname            -> " << it->first << '\n';
        std::cout << "\tUsername            -> " << it->second.getUsername() << '\n';
        std::cout << "\tNickname ref        -> " << &it->first << '\n';
    } */
}

void    printQueue(std::list<Connection>& conns)
{
    for (std::list<Connection>::iterator it = conns.begin(); it != conns.end(); ++it)
    {
        std::queue<std::string> msgQ = it->getQueue();
        std::cout << "Msg queue of " << it->getNickname() << '\n';
        std::cout << "MSg queue len " << msgQ.size() << " Front : " << msgQ.front() << '\n';
    }
}

void Server::pollEvents()
{
    int     ret;
    
    ret = poll(polls.data(), polls.size(), 2000);
    if (ret == -1)  
        throw (std::runtime_error(strerror(errno)));
    for (size_t i = 0; i < polls.size(); ++i)
    {
        if (polls.at(i).revents != 0)
        {
            if (i == 0)
                createConnection();
            else
                handleClientInteraction(polls.at(i));
        }
    }
    
    //printQueue(connections);
    //printserver();
}

void Server::handleClientInteraction(pollfd& activePoll)
{
    int             res;
    Connection& client = fdToConnection.find(activePoll.fd)->second;    

    if (activePoll.events & POLLHUP)
        std::cout << "client closed connection\n";
    if (activePoll.events & POLLOUT)
    {
        res = client.dequeueMsg();
        if (res < 0)
            std::cout << "Error : " << strerror(errno) << '\n';
    }
    else if (activePoll.events & POLLIN)
    {
        res = client.handleClientMsg();
        if (res == BUFFER_FULL)
            std::cout << "Todo : message bigger than 512 bytes\n"; //https://modern.ircdocs.horse/#errinputtoolong-417
        else if (res == READ_ERROR)
            std::cout << "Reading error : " << strerror(errno) << '\n';
        else if (res == CONNECTION_CLOSED)
        {
            notifyQuit(client.getNickname() + " closed connection\n", client);
            removeConnection(client);
        }
        else if (res == ENDLINE_RECEIVED)
            handleClientCommand(client, client.buffer);
    }
    else
        std::cout << "What? Event received -> " << activePoll.events << '\n';
}

void Server::handleClientCommand(Connection& client, const std::string& msg)
{
    std::stringstream           ss(msg);
    std::string                 line;
    std::string                 command;
    std::string                 arg("");
    std::vector<std::string>    args;

    client.clearBuffer();
    while (std::getline(ss, line))
    {
        std::stringstream s(line);
        s >> command;
        if (command == "CAP")
            continue;
        do
        {
            s >> arg;
            args.push_back(arg);
            arg.clear();
        } while (!s.eof());

        handleSimpleCommand(client, command, args);
        args.clear();
    }
    std::cout << "Message from the client\n" << msg << "\n";
}

void Server::handleSimpleCommand(Connection& client,
                                const std::string& cmd, 
                                const std::vector<std::string>& args)
{
    if (cmd == "NICK")
        CommandHandler::executeNick(args.at(0), client, nickToConnection, channels);
    else if (cmd == "USER")
        CommandHandler::executeUsername(args, client);
    else if (cmd == "PASS")
        CommandHandler::executePass(password, args.at(0), client);
    else if (!client.isRegistered()) //block here to prevent non registered clients from executing commands below
    {
        client.enqueueMsg(Replies::CommonErrReplies(client.getNickname(), "", ERR_NOTREGISTERED));
        return;
    }
    else if (cmd == "JOIN")
        CommandHandler::executeJoin(args, client, channels);
    else if (cmd == "PING")
        CommandHandler::executePing(args.at(0), client);
    else if (cmd == "PRIVMSG")
        CommandHandler::executePrivMsg(args, client, nickToConnection, channels);
    else if (cmd == "KICK")
        CommandHandler::executeKick(args, client, channels, nickToConnection);
    else if (cmd == "INVITE")
        CommandHandler::executeInvite(args, client, channels, nickToConnection);
    else if (cmd == "TOPIC")
        CommandHandler::executeTopic(args, client, channels);
    else if (cmd == "MODE")
        std::cout << "todo\n";
    else if (cmd == "QUIT")
    {
        notifyQuit(getReason(args), client);
        client.enqueueMsg("ERROR :You quit\r\n");
        removeConnection(client);
    }
}

void Server::registerConnection(Connection& newConnection, 
                                const pollfd& connectionPoll)
{
    polls.push_back(connectionPoll);
    newConnection.setConnectionPoll(&polls.back());
    connections.push_back(newConnection);
    fdToConnection.insert(std::pair<int, Connection&>(connectionPoll.fd, connections.back()));
}

void Server::createConnection()
{
    try
    {
        Connection      newConnection(listener.acceptConnection());
        struct pollfd   newPollFd;

        newPollFd.fd = newConnection.getFd();
        newPollFd.events = POLLIN;
        newPollFd.revents = 0;
        registerConnection(newConnection, newPollFd);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

void Server::removeConnection(Connection& client)
{
    removeClientFromChannels(client.getNickname());
    deregisterConnection(client);
}

void Server::removeClientFromChannels(const std::string& nickname)
{
    std::map<std::string, Channel>::iterator it;
    std::map<std::string, Channel>::iterator next;
    std::map<std::string, Channel>::iterator end;

    it = channels.begin();
    end = channels.end();
    next = it;
    for (; it != end; it = next)
    {
        ++next;
        if (it->second.isUserInChannel(nickname))
        {
            it->second.removeMember(nickname);
            if (it->second.isEmpty()) //delete channel if it's empty
                channels.erase(it);
        }
    }
}

void Server::deregisterConnection(Connection& client)
{
    std::list<Connection>::iterator it;
    std::list<Connection>::iterator end;
    int                             clientfd = client.getFd();
    const std::string               clientNickname = client.getNickname();

    nickToConnection.erase(client.getNickname());
    fdToConnection.erase(clientfd);
    it = connections.begin();
    end = connections.end();
    client.closeConnection();
    for (; it != end; ++it)
    {
        if (it->getNickname() == clientNickname)
        {
            connections.erase(it);
            break;
        }
    }
    for (std::vector<pollfd>::iterator pit = polls.begin(); pit != polls.end(); ++pit)
    {
        if (pit->fd == clientfd)
        {
            polls.erase(pit);
            break;
        }
    }
}

std::string getReason(const std::vector<std::string>& args)
{
    std::string theReason("");

    if (args.size() < 2)
        return (theReason);
    theReason = catArguments(args.begin(), args.end());
    return (theReason);
}

void Server::notifyQuit(const std::string& reason, const Connection& client) const
{
    const std::string& mask = client.getMask();

    std::string message = ":" + mask + " QUIT :Quit: " + reason + "\r\n";

    CommandHandler::notifyUsersInClientChannels(message, channels, client);
}