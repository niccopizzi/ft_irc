#include "../irc.hpp"

volatile sig_atomic_t serverShouldRun = 1;

void    startServer(char* port, char* password)
{
    Server  theServer(port, password);

    #ifdef LOG
    Logger logger("./ircserv.log");
    theServer.setLogger(&logger); 
    #endif

    try
    {
        theServer.openPort();
    }
    catch(const std::exception& e)
    {
        #ifdef LOG
            logger.log("Main ", std::string("Error in starting the server ") + e.what());
        #endif
        std::cerr << "Could not start server - Exiting...\n" << e.what();
        return;
    }
    registerSignalHandlers();
    std::cout << "\t\tSERVER STARTED!!!\n\tWE ARE RUNNING FAST AND FURIOUS ON PORT  : " << port << std::endl; 
    while (serverShouldRun)
    {
        try
        {
            theServer.pollEvents();
        }
        catch(const std::exception& e)
        {
            #ifdef LOG
                logger.log("Main", std::string("Caught the following exception : ") + e.what());
            #endif
        }
    }
    std::cout << "Received exit signal\n" << "--- CLOSING THE SERVER ---\n";
}