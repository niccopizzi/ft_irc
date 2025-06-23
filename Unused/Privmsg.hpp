#ifndef PRIVMSG_HPP
#define PRIVMSG_HPP

#include "Command.hpp"

class Privmsg : public Command
{
private:
    Privmsg(const Privmsg& privmsg);
    Privmsg& operator=(const Privmsg& other);
public:
    Privmsg();
    ~Privmsg();

    void execute(const std::vector<std::string>& args, 
                        Connection& client,
                        std::map<std::string, Connection&>& nickToConnection);
};

#endif
