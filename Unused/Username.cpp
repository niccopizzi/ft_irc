#include "Username.hpp"

Username::Username()
{
    #ifdef DEBUG
        std::cout << "Nick constructor called\n";
    #endif
}

/* Username::Username(const Username& username)
{
    #ifdef DEBUG
        std::cout << "Nick copy constructor called\n";
    #endif
}

Username& Username::operator=(const Username& other)
{
    #ifdef DEBUG
        std::cout << "Nick copy operator called\n";
    #endif
    return(*this);
} */

Username::~Username()
{
    #ifdef DEBUG
        std::cout << "Nick destructor called\n";
    #endif
}

void    setClientFullname(const std::vector<std::string>& args,
                            Connection& client)
{
    std::string fullname;

    for (std::vector<std::string>::const_iterator it = args.begin() + 3; 
            it != args.end(); ++it)
    {
        fullname += *it;
        if (it + 1 != args.end())
            fullname += " ";
    }
    if (fullname.at(0) == ':')
        fullname.erase(0, 1);
    client.setFullname(fullname);
}

void Username::execute(const std::vector<std::string>& args, 
                        Connection& client,
                        std::map<std::string, Connection&>& nickToConnection)
{
    (void)nickToConnection;
    if (!client.isAuthenticated())
        client.sendMessage("You need to authenticate first!\n");
    else if (args.size() < 4)
    {
        client.sendMessage(Replies::UserErrReplies(ERR_NEEDMOREPARAMS)); 
    }
    else if (client.isRegistered())
    {
        client.sendMessage(Replies::UserErrReplies(ERR_ALREADYREGISTERED));
    }
    else
    {
        client.setUsername(args.at(0));
        setClientFullname(args, client);
        if (client.isRegistered())
            client.sendMessage(Replies::WelcomeMsg(client.getNickname(), client.getUsername()));
    }
}