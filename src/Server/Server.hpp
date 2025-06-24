#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <sstream>
#include "Listener.hpp"
#include "../Commands/CommandHandler.hpp"
#include "../Channels/Channel.hpp"


#define HOSTNAME "Sambatime"
#define TIMEOUT_TIME 60

class Server
{
private:
    Listener                                listener;
    std::string                             password;
    std::vector<struct pollfd>              polls;
    std::map<std::string, Channel>          channels;
    std::map<int, Connection>               connections;
    std::map<std::string, Connection&>      nickToConnection;

    void    createConnection();
    void    notifyQuit(const std::string& message, Connection& client) const;
    void    removeConnection(Connection& client, int index);
    void    handleClientInteraction(Connection& client, int index);
    void    handleSimpleCommand(Connection& client, 
                                const std::string& cmd, 
                                const std::vector<std::string>& args,
                                int index);
    void    handleClientCommand(Connection& client, const std::string& msg, int index);
    
    void    printserver() const;

public:
    Server(); 
    Server(const char* port, const char* password);
    Server(const Server& server);
    Server& operator=(const Server& other);
    ~Server();
    
    void    openPort();
    void    pollEvents();

    Listener&                               getListener();
    std::map<std::string, Connection&>&     getNicksMap();
    std::map<int, Connection>&              getConnections();
    std::vector<struct pollfd>&             getPolls();
};

#endif // SERVER.HPP