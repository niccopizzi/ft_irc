#include "Bender.hpp"
#include "./bender_signals/bender_sig.hpp"
#include "./bender_types/BenderFactory.hpp"

volatile sig_atomic_t benderShouldRun = 1;

int main(int argc, char* argv[])
{
    benderArgs storage;
    
    if (!validateArgs(argc, argv, &storage))
        return(1);
    
    std::srand(time(NULL));
    
    registerSignalHandlers();
    Bender* bender = BenderFactory::getBender(storage.pass, storage.port, storage.lvl);
    try
    {
        bender->connectToServer();
        while (benderShouldRun)
        {
            bender->pollEvents();
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        if (bender != NULL)
            delete bender;
        return (1);
    }
    return (0);
}