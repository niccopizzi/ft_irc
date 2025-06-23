#ifndef USERNAME_HPP
#define USERNAME_HPP

#include "Command.hpp"

class Username : public Command
{
private:
    Username(const Username& username);
    Username& operator=(const Username& other);

public:
    Username();
    ~Username();

    virtual void execute(const std::vector<std::string>& args, 
                        Connection& client,
                        std::map<std::string, Connection&>& nickToConnection);
};

#endif