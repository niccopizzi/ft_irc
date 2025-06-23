#include "irc.hpp"

int main(int argc, char* argv[])
{
    if (!areArgsValid(argc, argv))
        return (2);
    startServer(argv[1], argv[2]);
    return (0);
}