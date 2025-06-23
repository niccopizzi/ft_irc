#include "Quit.hpp"

Quit::Quit()
{
    #ifdef DEBUG
        std::cout << "Quit constructor called\n";
    #endif
}

Quit::~Quit()
{
    #ifdef DEBUG
        std::cout << "Quit destructor called\n";
    #endif
}

void Quit::execute(const std::vector<std::string>& args, 
                Connection& client,
                std::map<std::string, Connection&>& nickToConnection)
{
    (void)args;
    nickToConnection.erase(client.getNickname());
    client.sendMessage("ERROR :Client quit\r\n");
    client.closeConnection();
}