#ifndef NICK_HPP
#define NICK_HPP

#include "Command.hpp"

class Nick : public Command
{
private:
    Nick(const Nick& nick);
    Nick& operator=(const Nick& other);

public:
    Nick();
    ~Nick();

    virtual void execute(const std::vector<std::string>& args, 
                        Connection& client,
                        std::map<std::string, Connection&>& nickToConnection);
};

#endif