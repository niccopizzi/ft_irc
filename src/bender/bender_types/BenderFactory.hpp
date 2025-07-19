#ifndef BENDERFACTORY_HPP
#define BENDERFACTORY_HPP

#include "BobBender.hpp"
#include "GandhiBender.hpp"
#include "SatanBender.hpp"

class BenderFactory
{
public:
    static Bender* getBender(const std::string& pass, const char* port,
                            const std::string& lvlOfHoliness);    
};

#endif