#ifndef PASS_HPP
#define PASS_HPP

#include "Command.hpp"

class Pass : public Command
{
private:
    std::string toMatch;
    Pass(const Pass& pass);
    Pass& operator=(const Pass& other);
public:
    Pass();
    Pass(const std::string& serverPass);
    ~Pass();

    void execute(const std::vector<std::string>& args, 
                    Connection& client,
                    std::map<std::string, Connection&>& nickToConnection);
};


#endif