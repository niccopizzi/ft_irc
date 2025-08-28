#include "Server.hpp"

Server::Server() : listener(),
                   password(""),
                   currentId(1)
{
    polls.reserve(50);
}

Server::Server(const char *port, const char *password) : listener(port),
                                                         password(password),
                                                         currentId(1)
{
    polls.reserve(50);
}

Server::Server(const Server &server) : listener(server.listener),
                                       password(server.password),
                                       polls(server.polls),
                                       connections(server.connections),
                                       channels(server.channels),
                                       fdToConnection(server.fdToConnection),
                                       nickToConnection(server.nickToConnection),
                                       currentId(server.currentId)
{
}

Server &Server::operator=(const Server &other)
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
        currentId = other.currentId;
    }
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
#ifdef LOG
    logger->log("server", " closing the server");
#endif
}

const Listener &Server::getListener() const
{
    return (listener);
}

const std::list<Connection> &Server::getConnections() const
{
    return (connections);
}

const std::vector<pollfd> &Server::getPolls() const
{
    return (polls);
}

const std::map<const std::string, Connection *> &Server::getNicksMap() const
{
    return (nickToConnection);
}

const std::map<int, Connection *> &Server::getFdMap() const
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
#ifdef LOG
    logger->log("server", "started listening");
#endif
}

void Server::pollEvents()
{
    int ret;

    ret = poll(polls.data(), polls.size(), -1);
    if (ret == -1)
        throw(std::runtime_error("Poll error"));
    for (size_t i = 0; i < polls.size(); ++i)
    {
        if (polls.at(i).revents != 0) //io event happened on fd
        {
            if (i == 0) //the first poll is the server listener and the rest are clients connection
                createConnection();
            else
                handleClientInteraction(polls.at(i));
        }
    }
}

void Server::handleClientInteraction(pollfd &activePoll)
{
    int res;
    Connection &client = *fdToConnection.find(activePoll.fd)->second;

    if (activePoll.events & POLLHUP)
    {
    #ifdef LOG
        logger->logConnection("Client hung up", client.getConnectionId());
    #endif
        notifyQuit(client.getNickname() + " closed connection", client);
        removeConnection(client);
    }
    if (activePoll.events & POLLOUT)
    {
        res = client.dequeueMsg();
    #ifdef LOG
        if (res < 0)
            logger->log("server", std::string("Error in sending message ") + strerror(errno));
    #endif
    }
    else if (activePoll.events & POLLIN)
    {
        res = client.handleClientMsg();
        if (res == BUFFER_FULL) //buffer here is cleared, send msg to client with error (https://modern.ircdocs.horse/#errinputtoolong-417)
            client.enqueueMsg(":localhost 417 :Input line was too long\r\n");    
        else if (res == READ_ERROR)
            removeConnection(client);
        else if (res == CONNECTION_CLOSED)
        {
            notifyQuit(client.getNickname() + " closed connection", client);
            removeConnection(client);
        }
        else if (res == ENDLINE_RECEIVED)
            handleClientCommand(client, client.buffer);
    }
    else
    {
        std::cout << "Unreachable";
    #ifdef LOG
        logger->log("server", "unrecognized poll event");
    #endif
    }
}

void Server::handleClientCommand(Connection &client, const std::string &msg)
{
    std::stringstream ss(msg);
    size_t pos;
    std::string line;
    std::string cmd;
    std::vector<std::string> args;

    client.clearBuffer();
    args.reserve(10);
    while (std::getline(ss, line))
    {
        if (*(line.end() - 1) == '\r')
            line.erase((line.end() - 1));
        pos = line.find(' ');
        if (pos != std::string::npos)
        {
            cmd = line.substr(0, pos);
            split(&(line.at(pos)), ' ', args);
            handleSimpleCommand(client, cmd, &args);
            args.clear();
        }
        else
        {
            handleSimpleCommand(client, line, NULL);
        }
    }
}

void Server::handleSimpleCommand(Connection &client,
                                 const std::string &cmd,
                                 const std::vector<std::string> *args)
{
#ifdef LOG
    if (args)
    {
        logger->logCommand(cmd, catArguments(args->begin(), args->end()), client.getConnectionId());
    }
    else
        logger->logCommand(cmd, "empty", client.getConnectionId());
#endif
    if (cmd == "CAP")
        return;
    if (cmd == "NICK")
        CommandHandler::executeNick(args, client, nickToConnection, channels);
    else if (cmd == "USER")
        CommandHandler::executeUsername(args, client);
    else if (cmd == "PASS")
        CommandHandler::executePass(args, client, password);
    else if (cmd == "QUIT")
    {
        if (client.isRegistered())
            notifyQuit(getReason(args), client);
        client.enqueueMsg("ERROR :You quit\r\n");
        removeConnection(client);
    }
    else if (!client.isRegistered()) // block here to prevent non registered clients from executing commands below
    {
        client.enqueueMsg(Replies::CommonErr(client.getNickname(), "", ERR_NOTREGISTERED));
        return;
    }
    else if (cmd == "JOIN")
        CommandHandler::executeJoin(args, client, channels);
    else if (cmd == "PING")
        CommandHandler::executePing(args, client);
    else if (cmd == "PRIVMSG")
        CommandHandler::executePrivMsg(args, client, channels, nickToConnection);
    else if (cmd == "KICK")
        CommandHandler::executeKick(args, client, channels, nickToConnection);
    else if (cmd == "INVITE")
        CommandHandler::executeInvite(args, client, channels, nickToConnection);
    else if (cmd == "TOPIC")
        CommandHandler::executeTopic(args, client, channels);
    else if (cmd == "MODE")
        CommandHandler::executeMode(args, client, channels, nickToConnection);
    else if (cmd == "LIST")
        CommandHandler::executeList(client, channels);
    else if (cmd == "WHO")
        CommandHandler::executeWho(args, client, channels);
}


bool Server::assignPollToConnection(Connection &newConnection)
{
    size_t  capacity;
    std::vector<pollfd>::iterator end(polls.end());
    pollfd  newPoll;

    capacity = polls.capacity();

    for (std::vector<pollfd>::iterator it = polls.begin() + 1; it != end; ++it)
    {
        if (it->fd == -1) //unusued poll, assign this one to the Connection instead of creating a new one
        {
            it->fd = newConnection.getFd();
            it->events = POLLIN;
            newConnection.setConnectionPoll(&(*it));
            return (false);
        }
    }
    newPoll.fd = newConnection.getFd();
    newPoll.events = POLLIN;
    newPoll.revents = 0;
    polls.push_back(newPoll);
    newConnection.setConnectionPoll(&polls.back());
    return (capacity == 0);
}

void Server::registerConnection(Connection &newConnection)
{
    std::vector<pollfd>::iterator end;
    bool    reallocationHappened;
    
    reallocationHappened = assignPollToConnection(newConnection);
    if (reallocationHappened) // reallocation happened need to store the new pointers, skip the last one because we already assigned it to the new Connection
    {
        end = (polls.end() - 1);
        for (std::vector<pollfd>::iterator it = polls.begin(); it != end; ++it)
        {
            Connection &mappedConn = *(fdToConnection.find(it->fd)->second);
            mappedConn.setConnectionPoll(&(*it));
        }
    }
    connections.push_back(newConnection);
    fdToConnection.insert(std::pair<int, Connection *>(newConnection.getFd(), &connections.back()));
}

void Server::createConnection()
{
    try
    {
        Connection newConnection(listener.acceptConnection());
        
        newConnection.setId(currentId);
        #ifdef LOG
            logger->logConnection("registered new connection", currentId);
            newConnection.setLogger(logger);
        #endif
        ++currentId;
        registerConnection(newConnection);
    }
    catch (const std::exception &e)
    {
        #ifdef LOG
            logger->log("server", std::string("exception in creating new Connection : ") + e.what());
        #endif
        std::cerr << e.what() << '\n';
    }
}

void Server::removeConnection(Connection &client)
{
    removeClientFromChannels(client.getConnectionId());
    deregisterConnection(client);
}

void Server::removeClientFromChannels(connectionID clientId)
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
        if (it->second.isUserInChannel(clientId))
        {
            #ifdef LOG
                logger->logConnection("removing from channel " + it->first, clientId);
            #endif
            it->second.removeMember(clientId);
            if (it->second.isEmpty()) // delete channel if it's empty
                channels.erase(it);
        }
    }
}

void Server::deregisterConnection(Connection &client)
{
    std::list<Connection>::iterator it;
    std::list<Connection>::iterator end;
    std::vector<pollfd>::iterator pend;
    int clientfd = client.getFd();
    const std::string clientNickname = client.getNickname();

    #ifdef LOG
        logger->logConnection("connection closed", client.getConnectionId());
    #endif
    nickToConnection.erase(clientNickname);
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
    pend = polls.end();
    for (std::vector<pollfd>::iterator pit = polls.begin() + 1; pit != pend; ++pit)
    {
        if (pit->fd == clientfd)
        {
            pit->fd = -1;
            pit->events = 0;
            pit->revents = 0;
            break;
        }
    }
}

std::string getReason(const std::vector<std::string> *args)
{
    std::string theReason("");

    if (args == NULL)
        return (theReason);
    return (catArguments(args->begin(), args->end()));
}

void Server::notifyQuit(const std::string &reason, const Connection &client) const
{
    const std::string &mask = client.getMask();

    std::string message = ":" + mask + " QUIT :Quit : " + reason + "\r\n";

    CommandHandler::notifyUsersInClientChannels(message, channels, client);
}

#ifdef LOG
void Server::setLogger(Logger* theLogger)
{
    logger = theLogger;
}
#endif