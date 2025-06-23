#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include "Listener.hpp"
#include "../Commands/CommandHandler.hpp"
#include "../Channels/Channel.hpp"

#define TIMEOUT_TIME 60

template<typename T>
void addToVector(std::vector<T>& vec, T& t)
{
    for (typename std::vector<T>::iterator it = vec.begin();
            it != vec.end(); ++it)
    {
        if (it->fd == -1)
        {
            *it = t;
            return;
        }
    }
    vec.push_back(t);
}

class Server
{
private:
    Listener                                listener;
    std::string                             password;
    std::vector<struct pollfd>              polls;
    std::vector<Connection>                 connections;
    std::vector<Channel>                    channels;
    std::map<std::string, Connection&>      nickToConnection;

    void    createConnection();
    void    removeConnection(Connection& client, int index);
    void    handleClientInteraction(Connection& client, int index);
    void    handleSimpleCommand(Connection& client, 
                                const std::string& cmd, 
                                const std::vector<std::string>& args);
    void    handleClientCommand(Connection& client, const std::string& msg);
    
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
    std::vector<Connection>&                getConnections();
    std::vector<struct pollfd>&             getPolls();
};

#endif // SERVER.HPP