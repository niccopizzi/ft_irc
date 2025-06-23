#ifndef QUIT_HPP
#define QUIT_HPP

#include "Command.hpp"

class Quit : public Command
{
private:
public:
    Quit();
    ~Quit();

    void execute(const std::vector<std::string>& args, 
                Connection& client,
                std::map<std::string, Connection&>& nickToConnection);
};



#endif //QUIT_HPP