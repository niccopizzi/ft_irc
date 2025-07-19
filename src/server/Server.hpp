#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <list>
#include <sstream>
#include "Listener.hpp"
#include "../commands/CommandHandler.hpp"
#include "../channel/Channel.hpp"


#define HOSTNAME "Sambatime"
#define TIMEOUT_TIME 60

class Server
{
private:
    Listener                                        listener;
    std::string                                     password;
    std::vector<pollfd>                             polls;
    std::list<Connection>                           connections;
    std::map<const std::string, Channel>            channels;
    std::map<int, Connection*>                      fdToConnection;
    std::map<const std::string, Connection*>        nickToConnection;
    connectionID                                    currentId;

    void    createConnection();
    void    registerConnection(Connection& newConnection,
                                const pollfd& connectionPoll);
    void    deregisterConnection(Connection& client);
    void    removeClientFromChannels(connectionID clientId);
    void    notifyQuit(const std::string& message, const Connection& client) const;
    void    removeConnection(Connection& client);
    void    handleClientInteraction(pollfd& activePoll);
    void    handleSimpleCommand(Connection& client,
                                const std::string& cmd,
                                const std::vector<std::string>* args);
    void    handleClientCommand(Connection& client, const std::string& msg);
    
    void    printserver() const;

public:
    Server(); 
    Server(const char* port, const char* password);
    Server(const Server& server);
    Server& operator=(const Server& other);
    ~Server();
    
    void    openPort();
    void    pollEvents();

    const Listener&                                     getListener() const;
    const std::list<Connection>&                        getConnections() const;
    const std::map<const std::string, Connection*>&     getNicksMap() const;
    const std::map<int, Connection*>&                   getFdMap()    const;
    const std::vector<pollfd>&                          getPolls()    const;
};

std::string getReason(const std::vector<std::string>* args);

#endif // SERVER.HPP