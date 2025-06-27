#include "Bender.hpp"

int main(int argc, char* argv[])
{
    Bender  bender("pass", "8080", GANDHI);

    (void)argc;
    (void)argv;
    try
    {
        bender.connectToServer();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return (1);
    }
    while (true)
    {
        try
        {
            bender.pollEvents();
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return (1);
        }
        
    }
    return (0);
}