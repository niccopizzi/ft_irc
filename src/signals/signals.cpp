#include "signals.hpp"


void    signalHandler(int signum)
{
    (void)signum;
    serverShouldRun = 0;
}

void    registerSignalHandlers()
{
    signal(SIGINT, signalHandler);
    signal(SIGQUIT, signalHandler);
}