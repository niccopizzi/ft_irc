#include "Server.hpp"

Server::Server() :  listener(), 
                    password("")
{
    polls.reserve(50);
    channels.reserve(50);
    #ifdef DEBUG
        std::cout << "Default Server constructor called\n";
    #endif
}

Server::Server(const char* port, const char* password) :    listener(port), 
                                                            password(password)
{
    polls.reserve(50);
    channels.reserve(50);
    #ifdef DEBUG
        std::cout << "Server constructor called\n";
    #endif
}

Server::Server(const Server& server) :  listener(server.listener), 
                                        password(server.password),
                                        polls(server.polls), 
                                        channels(server.channels),
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
        channels = other.channels;
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

Listener& Server::getListener()
{
    return (listener);
}

std::vector<Connection>& Server::getConnections()
{
    return (connections);
}

std::vector<struct pollfd>& Server::getPolls()
{
    return (polls);
}

std::map<std::string, Connection&>& Server::getNicksMap()
{
    return (nickToConnection);
}

void Server::openPort()
{
    struct pollfd listenerpoll;
    
    listener.createSocket(AF_UNSPEC, SOCK_STREAM);
    listener.startListen(SOMAXCONN);
    listenerpoll.fd = listener.getSocketFd();
    listenerpoll.events = POLLIN;
    listenerpoll.revents = 0;
    polls.push_back(listenerpoll);
}

void Server::pollEvents()
{
    int     ret;
    size_t  pollsSize;

    pollsSize = polls.size();
    ret = poll(polls.data(), pollsSize, 60);
    if (ret == -1)  
        throw (std::runtime_error(strerror(errno)));
    for (size_t i = 0; i < pollsSize; ++i)
    {
        if (polls.at(i).revents != 0)
        {
            if (i == 0)
                createConnection();
            else
                handleClientInteraction(connections.at(i - 1), i);
        }
    }
    //checkForTimeouts();
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
        
        addToVector(polls, newPollFd);
        addToVector(connections, newConnection);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

void Server::removeConnection(Connection& client, int index)
{
    polls.at(index).fd = -1;
    nickToConnection.erase(client.getNickname());
}

void Server::handleSimpleCommand(Connection& client, 
                                const std::string& cmd, 
                                const std::vector<std::string>& args)
{
    if (cmd == "NICK")
        CommandHandler::executeNick(args.at(0), client, nickToConnection);
    else if (cmd == "USER")
        CommandHandler::executeUsername(args, client);
    else if (cmd == "PASS")
        CommandHandler::executePass(password, args.at(0), client);
    else if (cmd == "PING")
        CommandHandler::executePing(args.at(0), client);
    else if (cmd == "PRIVMSG")
        CommandHandler::executePrivMsg(args, client, nickToConnection);
    else if (cmd == "QUIT")
        CommandHandler::executeQuit(args, client, nickToConnection);
    else if (cmd == "USER")
        CommandHandler::executeUsername(args, client);
}

void Server::handleClientCommand(Connection& client, const std::string& msg)
{
    std::stringstream           ss(msg);
    std::string                 line;
    std::string                 command;
    std::string                 arg("");
    std::vector<std::string>    args;

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
    }
    std::cout << "Message from the client\n" << msg << "\n";
    client.clearBuffer();
}

void Server::handleClientInteraction(Connection& client, int index)
{
    int     res;
    struct  pollfd& pollcl = polls.at(index);

    if (pollcl.events & POLLHUP)
        std::cout << "client closed connection\n";
    else if (pollcl.events & POLLIN)
    {    
        res = client.handleClientMsg();
        if (res == BUFFER_FULL)
            std::cout << "Todo : message bigger than 512 bytes\n"; //https://modern.ircdocs.horse/#errinputtoolong-417
        else if (res == READ_ERROR)
            std::cout << "Reading error : " << strerror(errno) << '\n';
        else if (res == CONNECTION_CLOSED)
        {
            removeConnection(client, index);
            client.closeConnection();
            std::cout << "connection closed\n";
            return;
        }
        else if (res == ENDLINE_RECEIVED)
            handleClientCommand(client, client.buffer);
    }
    if (client.fd == -1)
        polls.at(index).fd = -1;
}
