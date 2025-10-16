#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <list>
#include <cstdlib>
#include <sstream>
#include "Listener.hpp"
#include "CommandHandler.hpp"
#include "Channel.hpp"

#ifdef LOG
    #include "Logger.hpp"
#endif

#define HOSTNAME "Sambatime"
#define EVENT_TIMEOUT_TIME (1000)   // (ms) timeout every second
#define TIMEOUT_TIME (600)          // (s) timeout after 1 minute of inactivity
#define POLL_TIMEOUT_RET_VAL 0

class Server
{
private:

    Listener                                        listener;
    std::string                                     password;
    std::vector<pollfd>                             polls; // first listener, then clients
    std::list<Connection>                           connections;
    std::map<const std::string, Channel>            channels;
    std::map<int, Connection*>                      fdToConnection;
    std::map<const std::string, Connection*>        nickToConnection;
    connectionID                                    currentId;

    void    createConnection();
    void    registerConnection(Connection& newConnection);
    bool    assignPollToConnection(Connection &newConnection);
    void    deregisterConnection(Connection& client);
    void    removeClientFromChannels(connectionID clientId);
    void    notifyQuit(const std::string& reason,
                       const Connection& client) const;
    void    removeConnection(Connection& client);
    void    handleClientInteraction(pollfd& activePoll);
    void    handleSimpleCommand(Connection& client,
                                const std::string& cmd,
                                const std::vector<std::string>* args);
    void    handleClientCommand(Connection& client, const std::string& msg);
    void    checkForTimeouts();

    void    printserver() const;

#ifdef LOG
    Logger* logger;
#endif

public:

    // OCF
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

#ifdef LOG
    void setLogger(Logger* theLogger);
#endif

};

std::string getReason(const std::vector<std::string>* args);
