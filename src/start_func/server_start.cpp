#include "../irc.hpp"

volatile sig_atomic_t serverShouldRun = 1;

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
    registerSignalHandlers();
    std::cout << "SERVER STARTED!!!\nWE ARE RUNNING FAST AND FURIOUS ON PORT  : " << port << std::endl; 
    while (serverShouldRun)
    {
        try
        {
            theServer.pollEvents();
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        
    }
    std::cout << "Received exit signal\n" << "--- CLOSING THE SERVER ---\n";
}