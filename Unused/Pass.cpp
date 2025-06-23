#include "Pass.hpp"


Pass::Pass() : toMatch("")
{
    #ifdef DEBUG
        std::cout << "Pass default constructor called\n";
    #endif
}

Pass::Pass(const std::string& serverPass) : toMatch(serverPass)
{
    #ifdef DEBUG
        std::cout << "Pass constructor called\n";
    #endif
}

Pass::~Pass()
{
    #ifdef DEBUG
        std::cout << "Pass destructor called\n";
    #endif
}

void Pass::execute(const std::vector<std::string>& args, 
                    Connection& client,
                    std::map<std::string, Connection&>& nickToConnection)
{
    (void)nickToConnection;
    if (client.isAuthenticated())
    {
        client.sendMessage(Replies::PassErrReplies(ERR_ALREADYREGISTERED));
    }
    else if (args.empty())
    {
        client.sendMessage(Replies::PassErrReplies(ERR_NEEDMOREPARAMS));
    }
    else if (args.at(0) != toMatch)
    {
        client.sendMessage(Replies::PassErrReplies(ERR_PASSWDMISMATCH));
    }
    else
        client.setAuth(true);
}