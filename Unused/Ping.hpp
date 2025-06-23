#ifndef PING_HPP
#define PING_HPP

#include "Command.hpp"

class Ping : public Command
{
private:
public:
    Ping();
    ~Ping();

    void execute(const std::vector<std::string>& args, 
                        Connection& client,
                        std::map<std::string, Connection&>& nickToConnection);
};

#endif //PING_HPP