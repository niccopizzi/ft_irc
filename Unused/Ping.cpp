#include "Ping.hpp"

Ping::Ping()
{
    #ifdef DEBUG
        std::cout << "Ping default constructor called\n";
    #endif
}

Ping::~Ping()
{
    #ifdef DEBUG
        std::cout << "Ping destructor called\n";
    #endif
}

void Ping::execute(const std::vector<std::string>& args, 
                        Connection& client,
                        std::map<std::string, Connection&>& nickToConnection)
{
    (void)nickToConnection;
    if (!args.empty())
        client.sendMessage("PONG :" + args.at(0) + "\r\n");
    else
        client.sendMessage("PONG" "\r\n");
}