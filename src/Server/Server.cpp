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

std::map<int, Connection>& Server::getConnections()
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

void Server::printserver() const
{
    std::cout << "Printing server ---\n";
    
    std::cout << "CONNECTIONS\n";
    int index = 0;
    for (std::map<int, Connection>::const_iterator it = connections.begin();
        it != connections.end(); ++it)
    {
        std::cout << "\tConnection number : " << index <<'\n';
        std::cout << "\tConnection address : " << &(it->second) << '\n';
        std::cout << "\tfd                -> " << it->first << '\n';
        std::cout << "\tNickName          -> " << it->second.getNickname() << '\n';
        std::cout << "\tUsername          -> " << it->second.getUsername() << '\n';
        std::cout << "\tRegistered        -> " << (it->second.isRegistered() ? "Yes" : "No") << '\n';
        index++;
    }
    index = 0;
    std::cout << "\nCHANNELS\n";
    for (std::map<std::string, Channel>::const_iterator ct = channels.begin(); ct != channels.end(); ++ct)
    {
        std::cout << "\tChannel number : " << index <<'\n';
        std::cout << "\tChannel name   -> " << ct->first <<'\n';
        std::cout << "\tChannel members\n" <<'\n';
        int memnum = 0;
        const std::map<std::string, const Connection&>& members = ct->second.getMembers();
        for (std::map<std::string, const Connection&>::const_iterator member = members.begin();
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
    }
}

void Server::pollEvents()
{
    int     ret;
    size_t  pollsSize;

    pollsSize = polls.size();
    ret = poll(polls.data(), pollsSize, -1);
    if (ret == -1)  
        throw (std::runtime_error(strerror(errno)));
    for (size_t i = 0; i < pollsSize; ++i)
    {
        if (polls.at(i).revents != 0)
        {
            if (i == 0)
                createConnection();
            else
                handleClientInteraction(connections.find(polls.at(i).fd)->second, i);
        }
    }
    printserver();
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
        
        connections.insert(std::pair<int, Connection>(newPollFd.fd, newConnection));
        polls.push_back(newPollFd);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

void Server::removeConnection(Connection& client, int index)
{
    int clientFd;
    const std::string        clientNickname = client.getNickname();
    std::map<std::string, Channel>::iterator it;
    std::map<std::string, Channel>::iterator next;

    it = channels.begin();
    next = it;
    for (; it != channels.end(); it = next)
    {
        ++next;
        if (it->second.isUserInChannel(clientNickname))
        {
            it->second.removeMember(clientNickname);
            if (it->second.isEmpty()) //delete channel if it's empty
                channels.erase(it);
        }
    }
    clientFd = client.getFd();
    polls.erase(polls.begin() + index);
    client.closeConnection();
    nickToConnection.erase(clientNickname);
    connections.erase(clientFd);
}

std::string getReason(const std::vector<std::string>& args)
{
    std::string theReason("");

    if (args.size() < 2)
        return (theReason);
    theReason = catArguments(args.begin(), args.end());
    return (theReason);
}

void Server::handleSimpleCommand(Connection& client, 
                                const std::string& cmd, 
                                const std::vector<std::string>& args,
                                int index)
{
    if (cmd == "NICK")
        CommandHandler::executeNick(args.at(0), client, nickToConnection, channels);
    else if (cmd == "USER")
        CommandHandler::executeUsername(args, client);
    else if (cmd == "PASS")
        CommandHandler::executePass(password, args.at(0), client);
    else if (!client.isRegistered()) //block here to prevent non registered clients from executing commands below
    {
        client.sendMessage(Replies::CommonErrReplies(client.getNickname(), "", ERR_NOTREGISTERED));
        return;
    }
    else if (cmd == "JOIN")
        CommandHandler::executeJoin(args, client, channels);
    else if (cmd == "PING")
        CommandHandler::executePing(args.at(0), client);
    else if (cmd == "PRIVMSG")
        CommandHandler::executePrivMsg(args, client, nickToConnection, channels);
    else if (cmd == "KICK")
        std::cout << "todo\n";
    else if (cmd == "INVITE")
        std::cout << "todo\n";
    else if (cmd == "TOPIC")
        std::cout << "todo\n";
    else if (cmd == "MODE")
        std::cout << "todo\n";
    else if (cmd == "QUIT")
    {
        notifyQuit(getReason(args), client);
        client.sendMessage("ERROR :You quit\r\n");
        removeConnection(client, index);
    }
}

void Server::handleClientCommand(Connection& client, const std::string& msg, int index)
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

        handleSimpleCommand(client, command, args, index);
        args.clear();
    }
    std::cout << "Message from the client\n" << msg << "\n";
}

void Server::notifyQuit(const std::string& reason, Connection& client) const
{
    const std::string& mask = client.getMask();

    std::string message = ":" + mask + " QUIT :Quit: " + reason + "\r\n";

    CommandHandler::notifyUsersInClientChannels(message, channels, client);
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
            notifyQuit(client.getNickname() + " closed connection\n", client);
            removeConnection(client, index);
        }
        else if (res == ENDLINE_RECEIVED)
            handleClientCommand(client, client.buffer, index);
    }
    else
        std::cout << "What? Event received -> " << pollcl.events << '\n';
}
