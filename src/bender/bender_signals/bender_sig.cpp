#include "bender_sig.hpp"

void    signalHandler(int signum)
{
    (void)signum;
    benderShouldRun = 0;
}

void    registerSignalHandlers()
{
    signal(SIGINT, signalHandler);
    signal(SIGQUIT, signalHandler);
}