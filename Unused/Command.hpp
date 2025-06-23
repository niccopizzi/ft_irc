#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "../Replies/Replies.hpp"
#include "../Server/Connection.hpp"
#include <map>
#include <vector>

class Command
{
public:

    virtual ~Command() {};
    virtual void execute(const std::vector<std::string>& args, 
                        Connection& client,
                        std::map<std::string, Connection&>& nickToConnection) = 0;
};

#endif