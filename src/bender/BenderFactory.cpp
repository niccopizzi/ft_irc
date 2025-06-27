#include "BenderFactory.hpp"

Bender* BenderFactory::getBender(const std::string& pass, const char* port,
                            const std::string& lvlOfHoliness)
{
    if (lvlOfHoliness == "SATAN")
        return (new SatanBender(pass, port));
    else if (lvlOfHoliness == "BOB")
        return (new BobBender(pass, port));
    else if (lvlOfHoliness == "GANDHI")
        return (new GandhiBender(pass, port));
    else
        return (NULL);
}