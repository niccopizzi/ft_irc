#include "Server.hpp"

Server::Server() : listener(),
                   password(""),
                   currentId(1)
{ polls.reserve(50);}

Server::Server(const char *port, const char *password) : listener(port),
                                                         password(password),
                                                         currentId(1)
{ polls.reserve(50); }

Server::Server(const Server &server) : listener(server.listener),
                                       password(server.password),
                                       polls(server.polls),
                                       connections(server.connections),
                                       channels(server.channels),
                                       fdToConnection(server.fdToConnection),
                                       nickToConnection(server.nickToConnection),
                                       currentId(server.currentId)
{}

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

const Listener &Server::getListener() const { return (listener); }

const std::list<Connection> &Server::getConnections() const
{ return (connections); }

const std::vector<pollfd> &Server::getPolls() const { return (polls); }

const std::map<const std::string, Connection *> &Server::getNicksMap() const
{ return (nickToConnection); }

const std::map<int, Connection *> &Server::getFdMap() const
{ return (fdToConnection); }

/*
 * Opens listening TCP socket and registers it as first entry in polls array.
 * Binds/listens on it with system max backlog.
 * Builds a pollfd for that listener and registers it as an event source.
 */
void Server::openPort()
{
    pollfd listenerpoll;

    listener.createSocket(AF_UNSPEC, SOCK_STREAM); // SOCK_STREAM is stream-oriented socket that TCP uses
    listener.startListen(SOMAXCONN); // max
    listenerpoll.fd = listener.getSocketFd();
    listenerpoll.events = POLLIN;
    listenerpoll.revents = 0;
    polls.push_back(listenerpoll);
#ifdef LOG
    logger->log("server", "started listening");
#endif
}

void Server::checkForTimeouts()
{
    time_t  currTime = std::time(NULL);

    for (std::list<Connection>::iterator it = connections.begin();
        it != connections.end(); ++it)
    {
        if (currTime - it->getTimeOfLastInteraction() >= TIMEOUT_TIME)
        {
            std::cout << "Closing connection for client " << it->getFullname() << "\n";
            if (it->isRegistered())
                notifyQuit("Connection timeout", *it);
            it->enqueueMsg("ERROR :Timeout\r\n");
        }
    }
}

/*
 * Main loop that keeps IRC server running, accepting new connections and handling
 * client communication.
 * Polling is the POSIX system call to wait for 1 or more FDs to update.
 * Uses POSIX poll() system call to monitor multiple FDs (listening socket
 * and all connected client sockets).
 * polls is a vector of pollfd structs, each representing a socket. First is listening
 * socket and the rest are client sockets.
 */
void Server::pollEvents()
{
    int ret;

    ret = poll(polls.data(), polls.size(), EVENT_TIMEOUT_TIME); // blocks until >= 1 fds are ready
    if (ret == POLL_TIMEOUT_RET_VAL)
    {
        checkForTimeouts();
        return;
    }
    if (ret == -1)
        throw(std::runtime_error("Poll error"));
    for (size_t i = 0; i < polls.size(); i++)
    {
        // .at() is safer [] index operator
        if (polls.at(i).revents != 0) // io event happened on fd
        {
            // new client connection
            if (i == 0) // the first poll is the server listener and the rest are clients connection
                createConnection();
            else
                handleClientInteraction(polls.at(i));
        }
    }
    checkForTimeouts();
}

/*
 * This handles interaction from clients (not listening socket).
 */
void Server::handleClientInteraction(pollfd &activePoll)
{
    int res;
    // lookup corresponding connection
    Connection &client = *fdToConnection.find(activePoll.fd)->second;

    // hangup
    if (activePoll.events & POLLHUP)
    {
    #ifdef LOG
        logger->logConnection("Client hung up", client.getConnectionId());
    #endif
        notifyQuit("Client closed connection", client);
        removeConnection(client);
    }
    client.updateTimeOfLastInteraction();
    // POLLOUT is writeable, send any queued messages to client or handle error
    if (activePoll.events & POLLOUT)
    {
        res = client.dequeueMsg();
        if (res == ERROR_NOTIFIED)
            removeConnection(client);
        else if (res < 0)
        {
            notifyQuit("Client connection error", client);
            removeConnection(client);
            #ifdef LOG
                logger->log("server", std::string("Error in sending message ") + strerror(errno));
            #endif
        }
    }
    // checks POLLIN mask, call handleClientMsg() to read data from client.
    else if (activePoll.events & POLLIN)
    {
        res = client.handleClientMsg();
        if (res == BUFFER_FULL) // buffer here is cleared, send msg to client with error (https://modern.ircdocs.horse/#errinputtoolong-417)
            client.enqueueMsg(":localhost 417 :Input line was too long\r\n");    
        else if (res == READ_ERROR)
        {
            notifyQuit("Client connection error", client);
            #ifdef LOG
                logger->logConnection("read error", client.getConnectionId());
            #endif
            removeConnection(client);
        }
        else if (res == CONNECTION_CLOSED)
        {
            notifyQuit("Client closed connection", client);
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

/*
 * Parses client command using a string stream
 */
void Server::handleClientCommand(Connection &client, const std::string &msg)
{
    std::stringstream ss(msg);
    size_t pos;
    std::string line;
    std::string cmd;
    std::vector<std::string> args;

    client.clearBuffer();
    args.reserve(10);
    #ifdef LOG
        logger->logMessage("message from client", msg.substr(0, msg.size() - 2), client.getConnectionId());
    #endif
    while (std::getline(ss, line))
    {
        if (!std::isalpha(*line.begin()))
            return; // silently fail if line does not start with a char. IRC commands must start with a char
        if (*(line.end() - 1) == '\r')
            line.erase((line.end() - 1));
        pos = line.find(' ');
        if (pos != std::string::npos) // split args only if space is present
        {
            cmd = line.substr(0, pos);
            split(&(line.at(pos)), ' ', args);
            if (args.empty()) // handle case where after command there are only spaces and no args
                handleSimpleCommand(client, cmd, NULL);
            else
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
        logger->logCommand(cmd, catArguments(args->begin(), args->end()), client.getConnectionId());
    else
        logger->logCommand(cmd, "empty", client.getConnectionId());
#endif

    if (cmd == "CAP")
        return;

    if (cmd == "QUIT") {
        if (client.isRegistered())
            notifyQuit(getReason(args), client);
        client.enqueueMsg("ERROR :You quit\r\n");
        return;
    }

    // Only allow these commands if client is not registered
    if (!client.isRegistered() && cmd != "NICK" && cmd != "PASS" && cmd != "USER")
    {
        client.enqueueMsg(Replies::CommonErr(client.getNickname(), "", ERR_NOTREGISTERED));
        return;
    }

    // Command dispatch
    if (cmd == "NICK")
        CommandHandler::executeNick(args, client, nickToConnection, channels);
    else if (cmd == "USER")
        CommandHandler::executeUsername(args, client);
    else if (cmd == "PASS")
        CommandHandler::executePass(args, client, password);
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

/*
 * Assigns new or recycled pollfd to a new client.
 */
bool Server::assignPollToConnection(Connection &newConnection)
{
    size_t  needToReallocate;
    std::vector<pollfd>::iterator end(polls.end());
    pollfd  newPoll;

    for (std::vector<pollfd>::iterator it = polls.begin() + 1; it != end; ++it)
    {
        if (it->fd == -1) // unusued poll, assign this one to the Connection instead of creating a new one
        {
            it->fd = newConnection.getFd();
            it->events = POLLIN;
            newConnection.setConnectionPoll(&(*it));
            connections.push_back(newConnection);
            fdToConnection.insert(std::pair<int, Connection *>(newConnection.getFd(), &connections.back()));
            return (false);
        }
    }
    // if new pollfd must be created
    needToReallocate = (polls.capacity() <= (polls.size() + 1)); // check if reallocation is necessary
    newPoll.fd = newConnection.getFd();
    newPoll.events = POLLIN;
    newPoll.revents = 0;
    polls.push_back(newPoll);
    newConnection.setConnectionPoll(&polls.back());
    connections.push_back(newConnection);
    fdToConnection.insert(std::pair<int, Connection *>(newConnection.getFd(), &connections.back()));
    return (needToReallocate);
}

/*
 * In the event of reallocation, new pointers need to be stored
 */
void Server::registerConnection(Connection &newConnection)
{
    std::vector<pollfd>::iterator end;
    bool    reallocationHappened;

    reallocationHappened = assignPollToConnection(newConnection);
    if (reallocationHappened) // reallocation happened need to store the new pointers, skip the first one because is reserved for the server listener
    {
        end = (polls.end());
        for (std::vector<pollfd>::iterator it = polls.begin() + 1; it != end; ++it)
        {
            Connection* mappedConn = (fdToConnection.find(it->fd)->second);
            mappedConn->setConnectionPoll(&(*it));
        }
    }
}

/*
 * Accepts new client, wraps it in a Connection, assigns it ID, registers for
 * event monitoring and handles errors.
 */
void Server::createConnection()
{
    if (!listener.isOpen())
        return;

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

/*
 * When client disconnects, all references to them are removed, resources are
 * released and the server is ready to handle new connections. Clean up.
 */
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
    // client.getPollFd()->fd = -1;
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

    std::string message = ":" + mask + " QUIT :Quit: " + reason + "\r\n";

    CommandHandler::notifyUsersInClientChannels(message, channels, client);
}

#ifdef LOG
void Server::setLogger(Logger* theLogger) { logger = theLogger; }
#endif
