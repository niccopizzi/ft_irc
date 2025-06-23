#include "../irc.hpp"

void    startServer(char* port, char* password)
{
    Server  theServer(port, password);

    try
    {
        theServer.openPort();
    }

    catch(const std::exception& e)
    {
        std::cerr << "Fatal error occurred : " << e.what() << " exiting\n";
        return;
    }
    
    std::cout << "Running on port : " << port << std::endl; 
    while (true)
    {
        theServer.pollEvents();
    }
}