#include "irc.hpp"

bool    isPortValid(char* port)
{
    size_t  atoi;

    if (port[0] == 0)
    {
        std::cout << "Port cannot be empty\n";
        return (false);
    }
    atoi = 0;
    for (size_t i = 0; port[i] != 0; ++i)
    {
        if (!std::isdigit(port[i]))
        {
            std::cout << "Port cannot have non-digit values\n";
            return (false);
        }
        atoi = atoi * 10 + (port[i] - 48);
        if (atoi > __INT_MAX__)
        {
            std::cout << "Port number too big, please insert a smaller number\n";
            return (false);
        }
    }
    if (atoi < 1024)
    {
        std::cout << "Port cannot be below 1024\n";
        return (false);
    }
    return (true);
}

//check that there the port and the pass were given and that 
//the port is > 1024 (avoid using port less then 1024 because those ports
//are for the standard service) && < INT_MAX
//the pass must not be empty 

bool    areArgsValid(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cout << "ircserv usage : ./ircserv <port> <password>\n";
        return (false);
    }
    if (!isPortValid(argv[1]))
        return (false);
    if (argv[2][0] == 0)
    {
        std::cout << "Password cannot be empty\n";
        return (false);
    }
    return (true);
}