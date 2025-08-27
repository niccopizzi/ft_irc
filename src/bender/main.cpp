#include "Bender.hpp"
#include "./bender_types/BenderFactory.hpp"

int main(int argc, char* argv[])
{
    benderArgs storage;
    
    if (!validateArgs(argc, argv, &storage))
    return(1);
    
    std::srand(time(NULL));
    Bender* bender = BenderFactory::getBender(storage.pass, storage.port, storage.lvl);
    try
    {
        bender->connectToServer();
        while (true)
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